/*
 * =============================================================================
 *  meteo.ino  -  Stazione meteo fai-da-te (Arduino Nano)
 * =============================================================================
 *
 *  COSA FA
 *  -------
 *  Ogni 2 secondi legge i sensori e invia al PC (via USB) UNA riga di testo
 *  in formato JSON, per esempio:
 *
 *    {"id":"oss-meteo","v":1,"n":42,"ms":2000,"ta":11.52,"tc":-17.30,"mlx":1,"imp":3,"piog":0}
 *
 *    id   = nome del dispositivo (serve al PC per riconoscerlo)
 *    v    = versione del formato
 *    n    = numero progressivo della misura (se non cambia, l'Arduino e' bloccato)
 *    ms   = durata in millisecondi dell'intervallo in cui sono contati gli impulsi
 *    ta   = temperatura dell'aria (sensore interno dell'MLX90614), gradi C
 *    tc   = temperatura del cielo (infrarosso), gradi C
 *    mlx  = 1 se la lettura dell'MLX90614 e' valida, 0 se c'e' un problema
 *    imp  = impulsi dell'anemometro nell'intervallo
 *    piog = 1 se il sensore di pioggia segnala pioggia, 0 se asciutto
 *
 *  L'Arduino NON decide nulla: tutti i calcoli (velocita' del vento,
 *  nuvolosita') e le soglie sono nel programma Python sul PC, nel file
 *  di configurazione meteo.ini. Cosi' non serve riprogrammare l'Arduino
 *  per cambiare una soglia.
 *
 *  NESSUNA LIBRERIA DA INSTALLARE: l'MLX90614 viene letto direttamente con
 *  la libreria "Wire", gia' compresa nell'IDE di Arduino. Ogni lettura e'
 *  verificata con il codice di controllo (PEC) del sensore: se i dati
 *  arrivano rovinati (cavo lungo, disturbi) la misura viene scartata.
 *
 *  COLLEGAMENTI (dettagli in docs/03-collegamenti.md)
 *  ------------
 *   MLX90614 (modulo GY-906): VIN->5V, GND->GND, SDA->A4, SCL->A5
 *   Anemometro (contatto reed): un filo -> D2, l'altro -> GND
 *   Sensore pioggia (contatto del rele', CHIUSO = asciutto): D4 e GND
 *
 *  Licenza: MIT (vedi file LICENSE nel repository).
 * =============================================================================
 */

#include <Wire.h>

// ---- Configurazione ------------------------------------------------------------
#define BAUD_RATE          9600
#define INTERVALLO_MS      2000    // ogni quanto inviare una misura
#define PIN_ANEMOMETRO     2       // deve essere D2 o D3 (ingressi con interrupt)
#define PIN_PIOGGIA        4
#define INDIRIZZO_MLX      0x5A    // indirizzo standard dell'MLX90614
#define MLX_REG_ARIA       0x06    // registro temperatura ambiente
#define MLX_REG_CIELO      0x07    // registro temperatura oggetto (cielo)
// Impulsi piu' ravvicinati di cosi' sono rimbalzi del contatto
// (a 200 km/h un anemometro da 2,4 km/h per impulso/s da' un impulso ogni 12 ms).
#define ANTIRIMBALZO_ANEMOMETRO_MS 5
// Contatto pioggia: 1 = contatto CHIUSO significa asciutto (consigliato:
// un filo interrotto viene letto come "pioggia", cioe' dalla parte sicura).
#define PIOGGIA_CONTATTO_CHIUSO_ASCIUTTO 1

// ---- Variabili -------------------------------------------------------------------
volatile unsigned int impulsi = 0;
volatile unsigned long ultimoImpulsoMs = 0;
unsigned long numeroMisura = 0;
unsigned long inizioIntervallo = 0;

// Chiamata automaticamente a ogni giro dell'anemometro (interrupt).
void contaImpulso() {
  unsigned long ora = millis();
  if (ora - ultimoImpulsoMs >= ANTIRIMBALZO_ANEMOMETRO_MS) {
    impulsi++;
    ultimoImpulsoMs = ora;
  }
}

// Calcolo del codice di controllo CRC-8 (PEC) usato dal protocollo SMBus.
uint8_t crc8(uint8_t crc, uint8_t dato) {
  crc ^= dato;
  for (uint8_t i = 0; i < 8; i++) {
    crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
  }
  return crc;
}

// Legge un registro di temperatura dell'MLX90614.
// Restituisce true se la lettura e' valida; il risultato (gradi C) va in *gradi.
bool leggiMLX(uint8_t registro, float* gradi) {
  Wire.beginTransmission(INDIRIZZO_MLX);
  Wire.write(registro);
  if (Wire.endTransmission(false) != 0) return false;        // il sensore non risponde
  if (Wire.requestFrom((uint8_t)INDIRIZZO_MLX, (uint8_t)3) != 3) return false;
  uint8_t basso = Wire.read();
  uint8_t alto  = Wire.read();
  uint8_t pec   = Wire.read();

  uint8_t crc = 0;
  crc = crc8(crc, INDIRIZZO_MLX << 1);
  crc = crc8(crc, registro);
  crc = crc8(crc, (INDIRIZZO_MLX << 1) | 1);
  crc = crc8(crc, basso);
  crc = crc8(crc, alto);
  if (crc != pec) return false;                              // dati rovinati
  if (alto & 0x80) return false;                             // il sensore segnala errore

  unsigned int grezzo = ((unsigned int)alto << 8) | basso;
  *gradi = grezzo * 0.02 - 273.15;
  return true;
}

bool piove() {
  bool contattoChiuso = (digitalRead(PIN_PIOGGIA) == LOW);
#if PIOGGIA_CONTATTO_CHIUSO_ASCIUTTO
  return !contattoChiuso;
#else
  return contattoChiuso;
#endif
}

void setup() {
  pinMode(PIN_ANEMOMETRO, INPUT_PULLUP);
  pinMode(PIN_PIOGGIA, INPUT_PULLUP);
  Serial.begin(BAUD_RATE);
  Wire.begin();
  Wire.setClock(50000);                 // velocita' ridotta: piu' affidabile con cavi lunghi
  // Se il bus I2C si blocca (disturbi, cavo staccato) la libreria si sblocca
  // da sola dopo 25 ms invece di fermare l'Arduino per sempre.
  Wire.setWireTimeout(25000, true);
  attachInterrupt(digitalPinToInterrupt(PIN_ANEMOMETRO), contaImpulso, FALLING);
  inizioIntervallo = millis();
}

void loop() {
  unsigned long ora = millis();
  if (ora - inizioIntervallo < INTERVALLO_MS) return;

  // Prende e azzera il conteggio degli impulsi senza perderne nessuno.
  noInterrupts();
  unsigned int conteggio = impulsi;
  impulsi = 0;
  interrupts();
  unsigned long durata = ora - inizioIntervallo;
  inizioIntervallo = ora;

  float aria = 0, cielo = 0;
  bool mlxOk = leggiMLX(MLX_REG_ARIA, &aria);
  mlxOk = mlxOk && leggiMLX(MLX_REG_CIELO, &cielo);
  if (Wire.getWireTimeoutFlag()) {     // c'e' stato un blocco del bus: misura scartata
    Wire.clearWireTimeoutFlag();
    mlxOk = false;
  }

  numeroMisura++;
  Serial.print(F("{\"id\":\"oss-meteo\",\"v\":1,\"n\":"));
  Serial.print(numeroMisura);
  Serial.print(F(",\"ms\":"));
  Serial.print(durata);
  if (mlxOk) {
    Serial.print(F(",\"ta\":"));
    Serial.print(aria, 2);
    Serial.print(F(",\"tc\":"));
    Serial.print(cielo, 2);
    Serial.print(F(",\"mlx\":1"));
  } else {
    Serial.print(F(",\"ta\":null,\"tc\":null,\"mlx\":0"));
  }
  Serial.print(F(",\"imp\":"));
  Serial.print(conteggio);
  Serial.print(F(",\"piog\":"));
  Serial.print(piove() ? 1 : 0);
  Serial.println('}');
}

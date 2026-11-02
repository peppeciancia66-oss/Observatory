/*
 * =============================================================================
 *  tetto.ino  -  Controllore del tetto roll-off (Arduino Uno R3)
 * =============================================================================
 *
 *  COSA FA
 *  -------
 *  Questo programma gira sull'Arduino Uno collegato via USB al PC
 *  dell'osservatorio. Parla con il driver INDI "RollOff ino" (quello che
 *  Ekos usa per il tetto) e:
 *    - "preme" gli ingressi APRI / CHIUDI / STOP della scheda del cancello
 *      tramite tre rele' a contatto pulito (l'Arduino non tocca mai i
 *      morsetti della scheda);
 *    - legge i SUOI sensori (non i finecorsa interni del motore):
 *        finecorsa TETTO APERTO e TETTO CHIUSO,
 *        sensore di movimento (facoltativo),
 *        sensore fisico MONTATURA PARCHEGGIATA,
 *        contatto del pulsante di EMERGENZA,
 *        contatto del sensore di PIOGGIA;
 *    - ricava lo stato del tetto: aperto, chiuso, in apertura, in chiusura,
 *      fermo a meta', movimento non comandato.
 *
 *  SICUREZZE (tutte decise QUI, sull'Arduino, non sul PC)
 *  ------------------------------------------------------
 *   1. Il motore non parte se:
 *        - i finecorsa sono incoerenti (aperto e chiuso insieme);
 *        - la montatura NON e' parcheggiata (sia in apertura sia in chiusura);
 *        - il pulsante di emergenza e' premuto (o il suo filo e' interrotto);
 *        - c'e' un errore precedente non ancora "riconosciuto".
 *      In questi casi l'Arduino risponde al driver "LOCKED = ON": in Ekos si
 *      accende la luce rossa "Roof Lock" e il driver non invia comandi.
 *   2. Ogni movimento si ferma (impulso STOP) se:
 *        - supera il tempo massimo TEMPO_MAX_MOVIMENTO_S;
 *        - il tetto non parte entro TEMPO_MAX_DISTACCO_S;
 *        - (con il sensore di movimento) il tetto si ferma a meta';
 *        - durante il movimento la montatura esce dal parcheggio,
 *          si preme l'emergenza o i finecorsa diventano incoerenti;
 *        - il tetto arriva al finecorsa "sbagliato" (direzione invertita);
 *        - il PC smette di parlare per SILENZIO_MAX_IN_MOVIMENTO_S secondi.
 *   3. Se il tetto si muove SENZA che l'Arduino l'abbia comandato (telecomando,
 *      richiusura automatica della scheda rimasta attiva...) e la montatura
 *      NON e' parcheggiata, l'Arduino preme subito STOP (e lo ripete finche'
 *      il movimento continua). E' solo un avviso: il tetto non resta bloccato.
 *   4. Dopo un errore il tetto resta BLOCCATO finche' non riconnetti il tetto
 *      in Ekos (Disconnetti -> Connetti). E' voluto: prima vai a vedere.
 *   5. Un "watchdog" interno riavvia l'Arduino se il programma si pianta;
 *      al riavvio tutti i rele' sono a riposo (nessun tasto premuto).
 *   6. Chiusura autonoma per pioggia quando il PC non risponde:
 *      DISATTIVATA di default (CHIUSURA_AUTONOMA_PIOGGIA = 0).
 *
 *  I valori che dipendono dalla scheda del cancello (non ancora verificati)
 *  sono TUTTI nel file configurazione_scheda.h, nella stessa cartella.
 *
 *  PROTOCOLLO (deciso dal driver INDI, non modificabile)
 *  -----------------------------------------------------
 *   Il driver invia (senza a capo):     L'Arduino risponde:
 *     (CON:0:0)                           (ACK:0:versione)
 *     (GET:OPENED:0)                      (ACK:OPENED:ON)  oppure  OFF
 *     (GET:CLOSED:0)                      (ACK:CLOSED:ON|OFF)
 *     (GET:LOCKED:0)                      (ACK:LOCKED:ON|OFF)
 *     (GET:AUXSTATE:0)                    (ACK:AUXSTATE:ON|OFF)   = pioggia
 *     (SET:OPEN:ON)  (SET:CLOSE:ON)  (SET:ABORT:ON)
 *                                         (ACK:OPEN:ON) ...
 *   Errori:                               (NAK:ERROR:messaggio)
 *   Comandi in piu', usati solo da strumenti/prova_tetto.py:
 *     (INF:STATO:0)   -> (ACK:STATO:in apertura)   ecc.
 *     (INF:ERRORE:0)  -> (ACK:ERRORE:nessun errore) ecc.
 *
 *   NOTA TECNICA: il driver cerca la parola "ON" (maiuscola) dentro la
 *   risposta. Per questo tutti i testi sono scritti in minuscolo.
 *
 *  Licenza: MIT (vedi file LICENSE nel repository).
 * =============================================================================
 */

#include <avr/wdt.h>
#include "configurazione_scheda.h"

// =============================================================================
//  CONFIGURAZIONE GENERALE (non dipende dalla scheda del cancello)
// =============================================================================

#define VERSIONE   "oss-tetto-1.1"
#define BAUD_RATE  38400          // deve coincidere con Ekos (scheda Connection)

// ---- Uscite verso i rele' (pin dell'"Arduino 4 Relays Shield") ---------------
#define PIN_RELE_APRI      4
#define PIN_RELE_CHIUDI    7
#define PIN_RELE_STOP      8
#define PIN_RELE_RISERVA  12      // quarto rele': non usato, sempre a riposo

// ---- Ingressi -----------------------------------------------------------------
// Ogni sensore e' un contatto collegato tra il pin e GND (pull-up interno).
//   Contatto CHIUSO                 -> "attivo"
//   Contatto APERTO o FILO TAGLIATO -> "non attivo"
// Cosi' un filo interrotto non da' mai una falsa conferma.
#define PIN_SENSORE_MOVIMENTO        2  // contatto reed: impulsi al passaggio dei magneti
#define PIN_FC_APERTO               A0  // finecorsa NA: si chiude a tetto tutto aperto
#define PIN_FC_CHIUSO               A1  // finecorsa NA: si chiude a tetto tutto chiuso
#define PIN_MONTATURA_PARCHEGGIATA  A2  // contatti NA in serie: chiusi con montatura in parcheggio
#define PIN_PIOGGIA                 A3  // contatto del rele' pioggia: CHIUSO = asciutto
#define PIN_EMERGENZA               A4  // contatto NC del pulsante di emergenza
#define PIN_LED                     13  // LED sulla scheda Arduino

#define USA_INGRESSO_EMERGENZA  1
#define USA_SENSORE_PIOGGIA     1

// ---- Tempi (in secondi) --------------------------------------------------------
// Durante un movimento il driver interroga l'Arduino ogni secondo circa.
#define SILENZIO_MAX_IN_MOVIMENTO_S  15
// Da fermo il driver interroga ogni 3 secondi circa.
#define SILENZIO_PC_MUTO_S          120

// ---- Chiusura autonoma per pioggia (DISATTIVATA di default) -------------------
// Se 1: quando il PC e' muto da SILENZIO_PC_MUTO_S secondi e piove da
// PIOGGIA_CONFERMA_S secondi, l'Arduino chiude il tetto DA SOLO, ma solo se
// la montatura risulta parcheggiata e tutti i controlli sono a posto.
// Attivala solo dopo il collaudo completo (docs/07-collaudo.md, fase E).
#define CHIUSURA_AUTONOMA_PIOGGIA   0
#define PIOGGIA_CONFERMA_S         30

#define ANTIRIMBALZO_MS  50

// =============================================================================
//  DA QUI IN POI NON SERVE MODIFICARE NULLA
// =============================================================================

#if RELE_ATTIVO_BASSO
  #define RELE_ON   LOW
  #define RELE_OFF  HIGH
#else
  #define RELE_ON   HIGH
  #define RELE_OFF  LOW
#endif

enum Movimento { FERMO, APERTURA, CHIUSURA };

enum Errore {
  ERR_NESSUNO = 0,
  ERR_TEMPO_SCADUTO,
  ERR_NON_PARTE,
  ERR_DIREZIONE,
  ERR_SENSORI,
  ERR_MONTATURA,
  ERR_EMERGENZA,
  ERR_COMUNICAZIONE,
  ERR_FERMO_A_META,
  AVV_NON_COMANDATO     // solo avviso: NON blocca il tetto
};

// Messaggi in minuscolo (vedi NOTA TECNICA), in memoria flash.
const char msg0[] PROGMEM = "nessun errore";
const char msg1[] PROGMEM = "tempo massimo di movimento superato";
const char msg2[] PROGMEM = "il tetto non e' partito";
const char msg3[] PROGMEM = "finecorsa opposto raggiunto: direzione errata";
const char msg4[] PROGMEM = "finecorsa incoerenti (aperto e chiuso insieme)";
const char msg5[] PROGMEM = "montatura uscita dal parcheggio";
const char msg6[] PROGMEM = "arresto di emergenza";
const char msg7[] PROGMEM = "comunicazione col pc persa durante il movimento";
const char msg8[] PROGMEM = "il tetto si e' fermato a meta' durante il movimento";
const char msg9[] PROGMEM = "avviso: movimento non comandato fermato (montatura fuori parcheggio)";
const char* const MESSAGGI_ERRORE[] PROGMEM = {
  msg0, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9
};

// ---- Lettura con antirimbalzo ----------------------------------------------------
struct Ingresso {
  uint8_t pin;
  bool stabile;          // true = contatto chiuso (pin a GND)
  bool ultimoGrezzo;
  unsigned long tCambio;
};

Ingresso inAperto     = { PIN_FC_APERTO, false, false, 0 };
Ingresso inChiuso     = { PIN_FC_CHIUSO, false, false, 0 };
Ingresso inMontatura  = { PIN_MONTATURA_PARCHEGGIATA, false, false, 0 };
Ingresso inPioggia    = { PIN_PIOGGIA, false, false, 0 };
Ingresso inEmergenza  = { PIN_EMERGENZA, false, false, 0 };
Ingresso inMovimento  = { PIN_SENSORE_MOVIMENTO, false, false, 0 };

// ---- Variabili globali -------------------------------------------------------------
Movimento     movimento = FERMO;
Errore        errore = ERR_NESSUNO;
bool          movimentoAutonomo = false;   // true se avviato dall'Arduino (pioggia)
bool          distaccoAvvenuto = false;
bool          partitoDaFinecorsa = false;
unsigned long tInizioMovimento = 0;
unsigned long tFineMovimento = 0;
unsigned long tUltimoMessaggio = 0;
unsigned long tInizioPioggia = 0;
bool          pioggiaInCorso = false;
bool          tentativoAutonomoFatto = false;
// sensore di movimento e movimenti non comandati
unsigned long tUltimoImpulsoMovimento = 0;
bool          impulsoMaiVisto = true;
bool          eraAperto = false;
bool          eraChiuso = false;
unsigned long tUltimoMovimentoEsterno = 0;
bool          movimentoEsternoVisto = false;
bool          avvisoNonComandato = false;   // c'e' stato uno STOP per movimento non comandato

#define MAX_COMANDO 40
char bufferComando[MAX_COMANDO + 1];
uint8_t lunghezzaComando = 0;
bool dentroComando = false;

// =============================================================================
//  INGRESSI
// =============================================================================

void inizializzaIngresso(Ingresso &in) {
  pinMode(in.pin, INPUT_PULLUP);
  delay(2);
  bool v = (digitalRead(in.pin) == LOW);
  in.stabile = v;
  in.ultimoGrezzo = v;
  in.tCambio = millis();
}

// Restituisce true se il valore stabile e' appena cambiato.
bool aggiornaIngresso(Ingresso &in) {
  bool grezzo = (digitalRead(in.pin) == LOW);
  unsigned long ora = millis();
  if (grezzo != in.ultimoGrezzo) {
    in.ultimoGrezzo = grezzo;
    in.tCambio = ora;
  } else if (grezzo != in.stabile && (ora - in.tCambio) >= ANTIRIMBALZO_MS) {
    in.stabile = grezzo;
    return true;
  }
  return false;
}

bool tettoAperto()            { return inAperto.stabile; }
bool tettoChiuso()            { return inChiuso.stabile; }
bool montaturaParcheggiata()  { return inMontatura.stabile; }
bool finecorsaCoerenti()      { return !(tettoAperto() && tettoChiuso()); }

bool emergenzaOk() {
#if USA_INGRESSO_EMERGENZA
  return inEmergenza.stabile;          // contatto NC chiuso = tutto normale
#else
  return true;
#endif
}

bool piove() {
#if USA_SENSORE_PIOGGIA
  return !inPioggia.stabile;           // contatto chiuso = asciutto
#else
  return false;
#endif
}

// true se il sensore di movimento ha visto passare un magnete di recente.
bool movimentoRilevato() {
#if USA_SENSORE_MOVIMENTO
  return !impulsoMaiVisto &&
         (millis() - tUltimoImpulsoMovimento) < (unsigned long)FINESTRA_MOVIMENTO_MS;
#else
  return false;
#endif
}

void aggiornaTuttiGliIngressi() {
  aggiornaIngresso(inAperto);
  aggiornaIngresso(inChiuso);
  aggiornaIngresso(inMontatura);
  aggiornaIngresso(inPioggia);
  aggiornaIngresso(inEmergenza);
  // Un impulso = il contatto si chiude (magnete che arriva).
  if (aggiornaIngresso(inMovimento) && inMovimento.stabile) {
    tUltimoImpulsoMovimento = millis();
    impulsoMaiVisto = false;
  }
}

bool movimentoConsentito() {
  return errore == ERR_NESSUNO
      && emergenzaOk()
      && montaturaParcheggiata()
      && finecorsaCoerenti();
}

// =============================================================================
//  RELE'
// =============================================================================

void tuttiReleARiposo() {
  digitalWrite(PIN_RELE_APRI, RELE_OFF);
  digitalWrite(PIN_RELE_CHIUDI, RELE_OFF);
  digitalWrite(PIN_RELE_STOP, RELE_OFF);
  digitalWrite(PIN_RELE_RISERVA, RELE_OFF);
}

// "Preme e rilascia" un ingresso della scheda del cancello.
void impulso(uint8_t pinRele) {
  tuttiReleARiposo();                  // mai due comandi insieme
  digitalWrite(pinRele, RELE_ON);
  delay(IMPULSO_MS);                   // breve: il watchdog (2 s) non scatta
  digitalWrite(pinRele, RELE_OFF);
  delay(PAUSA_DOPO_IMPULSO_MS);
  wdt_reset();
}

void premiStop() {
  impulso(PIN_RELE_STOP);
}

void fineMovimento() {
  movimento = FERMO;
  movimentoAutonomo = false;
  tFineMovimento = millis();
  eraAperto = tettoAperto();
  eraChiuso = tettoChiuso();
}

// Ferma il tetto e (se err != ERR_NESSUNO) registra un errore bloccante.
void arresto(Errore err) {
  premiStop();
  fineMovimento();
  if (err != ERR_NESSUNO) {
    errore = err;
  }
}

// =============================================================================
//  COMANDI DI MOVIMENTO
// =============================================================================

// Restituisce NULL se il movimento e' partito (o non serviva), altrimenti
// un messaggio che spiega perche' e' stato rifiutato.
const __FlashStringHelper* avviaMovimento(Movimento direzione, bool autonomo) {
  if (movimento != FERMO) {
    if (movimento == direzione) return NULL;           // gia' in corso
    arresto(ERR_NESSUNO);
    return F("tetto in movimento opposto: fermato, ripetere il comando");
  }
  if (errore != ERR_NESSUNO)    return F("errore precedente: riconnettere il tetto in ekos");
  if (!emergenzaOk())           return F("arresto di emergenza attivo");
  if (!montaturaParcheggiata()) return F("montatura non parcheggiata");
  if (!finecorsaCoerenti())     return F("finecorsa incoerenti");

  if (direzione == APERTURA && tettoAperto()) return NULL;   // gia' aperto
  if (direzione == CHIUSURA && tettoChiuso()) return NULL;   // gia' chiuso

  partitoDaFinecorsa = (direzione == APERTURA) ? tettoChiuso() : tettoAperto();
  distaccoAvvenuto = !partitoDaFinecorsa;

  impulso(direzione == APERTURA ? PIN_RELE_APRI : PIN_RELE_CHIUDI);
  movimento = direzione;
  movimentoAutonomo = autonomo;
  tInizioMovimento = millis();
  return NULL;
}

// Controlli continui durante un movimento comandato.
void sorvegliaMovimento() {
  if (movimento == FERMO) return;

  unsigned long trascorso = millis() - tInizioMovimento;
  bool oltreDistacco = trascorso > (unsigned long)TEMPO_MAX_DISTACCO_S * 1000UL;

  if (!emergenzaOk())           { arresto(ERR_EMERGENZA); return; }
  if (!montaturaParcheggiata()) { arresto(ERR_MONTATURA); return; }
  if (!finecorsaCoerenti())     { arresto(ERR_SENSORI);   return; }

  // Arrivato a destinazione?
  if ((movimento == APERTURA && tettoAperto()) ||
      (movimento == CHIUSURA && tettoChiuso())) {
#if FERMA_AL_FINECORSA
    premiStop();
#endif
    fineMovimento();
    return;
  }

  // Il tetto si e' staccato dal finecorsa di partenza?
  if (!distaccoAvvenuto) {
    bool ancoraSulFinecorsa = (movimento == APERTURA) ? tettoChiuso() : tettoAperto();
    if (!ancoraSulFinecorsa) {
      distaccoAvvenuto = true;
    } else if (oltreDistacco) {
      arresto(ERR_NON_PARTE);
      return;
    }
  } else if ((movimento == APERTURA && tettoChiuso()) ||
             (movimento == CHIUSURA && tettoAperto())) {
    // Finecorsa OPPOSTO a quello voluto: direzione sbagliata.
    arresto(ERR_DIREZIONE);
    return;
  }

#if USA_SENSORE_MOVIMENTO
  // Con il sensore di movimento: dopo l'avvio, i magneti devono passare.
  if (oltreDistacco && !movimentoRilevato()) {
    // Si e' mosso almeno un po'? (un magnete passato dopo l'avvio, oppure
    // il finecorsa di partenza lasciato)
    bool impulsoDopoAvvio = !impulsoMaiVisto &&
        (long)(tUltimoImpulsoMovimento - tInizioMovimento) >= 0;
    bool siEMosso = impulsoDopoAvvio || (partitoDaFinecorsa && distaccoAvvenuto);
    arresto(siEMosso ? ERR_FERMO_A_META : ERR_NON_PARTE);
    return;
  }
#endif

  if (trascorso > (unsigned long)TEMPO_MAX_MOVIMENTO_S * 1000UL) {
    arresto(ERR_TEMPO_SCADUTO);
    return;
  }

  if (!movimentoAutonomo &&
      (millis() - tUltimoMessaggio) > (unsigned long)SILENZIO_MAX_IN_MOVIMENTO_S * 1000UL) {
    arresto(ERR_COMUNICAZIONE);
    return;
  }
}

// Movimenti che l'Arduino NON ha comandato (telecomando, richiusura automatica
// della scheda, tetto spinto a mano...).
void sorvegliaMovimentiNonComandati() {
  if (movimento != FERMO) return;
  bool lasciatoFinecorsa = (eraAperto && !tettoAperto()) || (eraChiuso && !tettoChiuso());
  eraAperto = tettoAperto();
  eraChiuso = tettoChiuso();

  if ((millis() - tFineMovimento) < (unsigned long)TOLLERANZA_DOPO_MOVIMENTO_MS) return;

  if (lasciatoFinecorsa || movimentoRilevato()) {
    movimentoEsternoVisto = true;
    tUltimoMovimentoEsterno = millis();
    // STOP, ripetuto ogni TOLLERANZA_DOPO_MOVIMENTO_MS finche' il movimento
    // continua. NON e' un errore bloccante: una vibrazione del finecorsa
    // durante l'osservazione non deve impedire la chiusura successiva.
    if (!montaturaParcheggiata()) {
      arresto(ERR_NESSUNO);
      avvisoNonComandato = true;
    }
  }
}

// true se negli ultimi istanti c'e' stato un movimento non comandato.
bool movimentoEsternoRecente() {
  if (!movimentoEsternoVisto) return false;
  if (movimentoRilevato()) return true;
  return (millis() - tUltimoMovimentoEsterno) < (unsigned long)FINESTRA_MOVIMENTO_MS;
}

// Chiusura autonoma per pioggia con PC muto (solo se abilitata).
void controllaPioggiaAutonoma() {
  unsigned long ora = millis();

  if (piove()) {
    if (!pioggiaInCorso) {
      pioggiaInCorso = true;
      tInizioPioggia = ora;
    }
  } else {
    pioggiaInCorso = false;
    tentativoAutonomoFatto = false;
  }

#if CHIUSURA_AUTONOMA_PIOGGIA
  bool pcMuto = (ora - tUltimoMessaggio) > (unsigned long)SILENZIO_PC_MUTO_S * 1000UL;
  bool pioggiaConfermata = pioggiaInCorso &&
                           (ora - tInizioPioggia) > (unsigned long)PIOGGIA_CONFERMA_S * 1000UL;
  if (pcMuto && pioggiaConfermata && !tentativoAutonomoFatto &&
      movimento == FERMO && !tettoChiuso() && movimentoConsentito()) {
    tentativoAutonomoFatto = true;     // un solo tentativo per episodio
    avviaMovimento(CHIUSURA, true);
  }
#endif
}

// Stato del tetto in parole (minuscole).
const __FlashStringHelper* descriviStato() {
  if (movimento == APERTURA)            return F("in apertura");
  if (movimento == CHIUSURA)            return F("in chiusura");
  if (!finecorsaCoerenti())             return F("finecorsa incoerenti");
  if (movimentoEsternoRecente())        return F("movimento non comandato");
  if (tettoAperto())                    return F("aperto");
  if (tettoChiuso())                    return F("chiuso");
  return F("fermo a meta'");
}

// =============================================================================
//  COMUNICAZIONE CON IL DRIVER INDI
// =============================================================================

void rispondi(const char* tipo, const char* oggetto, const char* valore) {
  Serial.print('(');
  Serial.print(tipo);
  Serial.print(':');
  Serial.print(oggetto);
  Serial.print(':');
  Serial.print(valore);
  Serial.println(')');
}

void rispondiAck(const char* oggetto, bool acceso) {
  rispondi("ACK", oggetto, acceso ? "ON" : "OFF");
}

void rispondiAckTesto(const char* oggetto, const __FlashStringHelper* testo) {
  Serial.print(F("(ACK:"));
  Serial.print(oggetto);
  Serial.print(':');
  Serial.print(testo);
  Serial.println(')');
}

void rispondiNak(const __FlashStringHelper* messaggio) {
  Serial.print(F("(NAK:ERROR:"));
  Serial.print(messaggio);
  Serial.println(')');
}

void eseguiComando(char* testo) {
  char* comando = strtok(testo, ":");
  char* oggetto = strtok(NULL, ":");
  char* valore  = strtok(NULL, ":");
  if (comando == NULL || oggetto == NULL || valore == NULL) {
    rispondiNak(F("comando non valido"));
    return;
  }

  tUltimoMessaggio = millis();         // il PC e' vivo

  // ---- Connessione iniziale: azzera gli errori (riconoscimento) ----
  if (strcmp(comando, "CON") == 0) {
    errore = ERR_NESSUNO;
    movimentoEsternoVisto = false;
    avvisoNonComandato = false;
    rispondi("ACK", "0", VERSIONE);
    return;
  }

  if (strcmp(comando, "GET") == 0) {
    if      (strcmp(oggetto, "OPENED") == 0)   rispondiAck(oggetto, tettoAperto());
    else if (strcmp(oggetto, "CLOSED") == 0)   rispondiAck(oggetto, tettoChiuso());
    else if (strcmp(oggetto, "LOCKED") == 0)   rispondiAck(oggetto, movimento == FERMO && !movimentoConsentito());
    else if (strcmp(oggetto, "AUXSTATE") == 0) rispondiAck(oggetto, piove());
    else rispondiNak(F("richiesta sconosciuta"));
    return;
  }

  if (strcmp(comando, "SET") == 0) {
    bool acceso = (strcmp(valore, "ON") == 0);

    if (strcmp(oggetto, "OPEN") == 0 || strcmp(oggetto, "CLOSE") == 0) {
      if (!acceso) { rispondiAck(oggetto, false); return; }
      Movimento dir = (strcmp(oggetto, "OPEN") == 0) ? APERTURA : CHIUSURA;
      const __FlashStringHelper* rifiuto = avviaMovimento(dir, false);
      if (rifiuto == NULL) rispondiAck(oggetto, true);
      else                 rispondiNak(rifiuto);
      return;
    }
    if (strcmp(oggetto, "ABORT") == 0) {
      if (acceso) {
        if (movimento != FERMO || STOP_ANCHE_DA_FERMO) arresto(ERR_NESSUNO);
      }
      rispondiAck(oggetto, acceso);
      return;
    }
    if (strcmp(oggetto, "LOCK") == 0 || strcmp(oggetto, "AUXSET") == 0) {
      rispondiAck(oggetto, acceso);       // non usati: si conferma e basta
      return;
    }
    rispondiNak(F("comando sconosciuto"));
    return;
  }

  // Comandi di diagnostica per strumenti/prova_tetto.py (il driver non li usa)
  if (strcmp(comando, "INF") == 0) {
    if (strcmp(oggetto, "STATO") == 0) {
      rispondiAckTesto("STATO", descriviStato());
    } else if (strcmp(oggetto, "ERRORE") == 0) {
      char testoErrore[80];
      Errore daMostrare = errore;
      if (daMostrare == ERR_NESSUNO && avvisoNonComandato) daMostrare = AVV_NON_COMANDATO;
      strncpy_P(testoErrore, (PGM_P)pgm_read_word(&(MESSAGGI_ERRORE[daMostrare])), sizeof(testoErrore) - 1);
      testoErrore[sizeof(testoErrore) - 1] = '\0';
      rispondi("ACK", "ERRORE", testoErrore);
    } else {
      rispondiNak(F("richiesta sconosciuta"));
    }
    return;
  }

  rispondiNak(F("comando sconosciuto"));
}

void leggiSeriale() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '(') {
      dentroComando = true;
      lunghezzaComando = 0;
    } else if (c == ')') {
      if (dentroComando) {
        bufferComando[lunghezzaComando] = '\0';
        eseguiComando(bufferComando);
      }
      dentroComando = false;
      lunghezzaComando = 0;
    } else if (dentroComando) {
      if (lunghezzaComando < MAX_COMANDO) {
        bufferComando[lunghezzaComando++] = c;
      } else {
        dentroComando = false;
        rispondiNak(F("comando troppo lungo"));
      }
    }
  }
}

// =============================================================================
//  LED DI STATO
//    acceso fisso         -> tetto in movimento comandato
//    lampeggio veloce     -> errore bloccante
//    lampeggio lento      -> movimento non consentito (es. montatura fuori parcheggio)
//    spento               -> tutto pronto
// =============================================================================
void aggiornaLed() {
  unsigned long ora = millis();
  bool acceso;
  if (movimento != FERMO)           acceso = true;
  else if (errore != ERR_NESSUNO)   acceso = (ora / 150) % 2;
  else if (!movimentoConsentito())  acceso = (ora / 800) % 2;
  else                              acceso = false;
  digitalWrite(PIN_LED, acceso ? HIGH : LOW);
}

// =============================================================================
//  SETUP E LOOP
// =============================================================================

void setup() {
  // Rele' a riposo PRIMA di dichiarare i pin come uscite: all'accensione
  // nessun rele' "scatta".
  digitalWrite(PIN_RELE_APRI, RELE_OFF);
  digitalWrite(PIN_RELE_CHIUDI, RELE_OFF);
  digitalWrite(PIN_RELE_STOP, RELE_OFF);
  digitalWrite(PIN_RELE_RISERVA, RELE_OFF);
  pinMode(PIN_RELE_APRI, OUTPUT);
  pinMode(PIN_RELE_CHIUDI, OUTPUT);
  pinMode(PIN_RELE_STOP, OUTPUT);
  pinMode(PIN_RELE_RISERVA, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  // Stato iniziale esplicito (utile anche per le prove al computer).
  movimento = FERMO;
  errore = ERR_NESSUNO;
  movimentoAutonomo = false;
  impulsoMaiVisto = true;
  tUltimoImpulsoMovimento = 0;
  movimentoEsternoVisto = false;
  avvisoNonComandato = false;
  pioggiaInCorso = false;
  tentativoAutonomoFatto = false;
  dentroComando = false;
  lunghezzaComando = 0;

  inizializzaIngresso(inAperto);
  inizializzaIngresso(inChiuso);
  inizializzaIngresso(inMontatura);
  inizializzaIngresso(inPioggia);
  inizializzaIngresso(inEmergenza);
  inizializzaIngresso(inMovimento);
  eraAperto = tettoAperto();
  eraChiuso = tettoChiuso();

  Serial.begin(BAUD_RATE);
  tUltimoMessaggio = millis();
  tFineMovimento = millis();

  wdt_enable(WDTO_2S);   // se il programma si blocca per 2 s, l'Arduino si riavvia
}

void loop() {
  wdt_reset();
  aggiornaTuttiGliIngressi();
  leggiSeriale();
  sorvegliaMovimento();
  sorvegliaMovimentiNonComandati();
  controllaPioggiaAutonoma();
  aggiornaLed();
}

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
 *    - "preme" i tasti APRI / CHIUDI / STOP della scheda del cancello
 *      tramite tre rele' (come farebbe il telecomando);
 *    - legge i finecorsa TETTO APERTO e TETTO CHIUSO;
 *    - legge il sensore fisico MONTATURA PARCHEGGIATA;
 *    - legge il contatto del pulsante di EMERGENZA;
 *    - legge (facoltativo) il contatto del sensore di PIOGGIA.
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
 *        - il tetto non si stacca dal finecorsa di partenza entro
 *          TEMPO_MAX_DISTACCO_S (motore che non parte);
 *        - durante il movimento la montatura esce dal parcheggio,
 *          si preme l'emergenza o i finecorsa diventano incoerenti;
 *        - il tetto arriva al finecorsa "sbagliato" (direzione invertita);
 *        - il PC smette di parlare per SILENZIO_MAX_IN_MOVIMENTO_S secondi
 *          (PC bloccato, cavo USB staccato, Ekos chiuso).
 *   3. Dopo uno di questi errori il tetto resta BLOCCATO finche' non
 *      riconnetti il tetto in Ekos (Disconnetti -> Connetti). E' voluto:
 *      prima devi andare a vedere cosa e' successo.
 *   4. Un "watchdog" interno riavvia l'Arduino se il programma si pianta;
 *      al riavvio tutti i rele' sono a riposo (nessun tasto premuto).
 *   5. Chiusura autonoma per pioggia quando il PC non risponde:
 *      DISATTIVATA di default (CHIUSURA_AUTONOMA_PIOGGIA = 0).
 *
 *  ATTENZIONE: le parti marcate "DA VERIFICARE DOPO LA FOTO DELLA SCHEDA"
 *  dipendono dalla scheda del cancello e NON vanno considerate definitive.
 *
 *  PROTOCOLLO (deciso dal driver INDI, non modificabile)
 *  -----------------------------------------------------
 *   Il driver invia (senza a capo):     L'Arduino risponde:
 *     (CON:0:0)                           (ACK:0:versione)
 *     (GET:OPENED:0)                      (ACK:OPENED:ON)  oppure  OFF
 *     (GET:CLOSED:0)                      (ACK:CLOSED:ON|OFF)
 *     (GET:LOCKED:0)                      (ACK:LOCKED:ON|OFF)
 *     (GET:AUXSTATE:0)                    (ACK:AUXSTATE:ON|OFF)
 *     (SET:OPEN:ON)  (SET:CLOSE:ON)  (SET:ABORT:ON)
 *                                         (ACK:OPEN:ON) ...
 *   Errori:                               (NAK:ERROR:messaggio)
 *
 *   NOTA TECNICA: il driver cerca la parola "ON" (maiuscola) dentro la
 *   risposta. Per questo i messaggi di errore sono scritti tutti in
 *   minuscolo: un "ON" maiuscolo dentro un messaggio verrebbe frainteso.
 *
 *  Licenza: MIT (vedi file LICENSE nel repository).
 * =============================================================================
 */

#include <avr/wdt.h>

// =============================================================================
//  CONFIGURAZIONE  (le uniche righe che normalmente si modificano)
// =============================================================================

#define VERSIONE   "oss-tetto-1.0"
#define BAUD_RATE  38400          // deve coincidere con Ekos (scheda Connection)

// ---- Uscite verso i rele' ---------------------------------------------------
// Pin uguali a quelli dell'"Arduino 4 Relays Shield" (4, 7, 8, 12): vanno
// bene anche con un modulo a 4 rele' separato.
#define PIN_RELE_APRI      4
#define PIN_RELE_CHIUDI    7
#define PIN_RELE_STOP      8
#define PIN_RELE_RISERVA  12      // quarto rele': non usato, sempre a riposo

// La maggior parte dei moduli rele' economici con optoisolatori si attiva
// con il pin a livello BASSO ("attivo basso"): in quel caso lascia 1.
// Lo Shield ufficiale Arduino e' invece "attivo alto": metti 0.
// Lo verificherai al collaudo (fase A) senza nulla collegato ai contatti.
#define RELE_ATTIVO_BASSO  1

// Durata della "pressione del tasto" in millisecondi.
// DA VERIFICARE DOPO LA FOTO DELLA SCHEDA (di solito 300-1000 ms).
#define IMPULSO_MS             500
#define PAUSA_DOPO_IMPULSO_MS  300

// Se 1, quando il tetto raggiunge il NOSTRO finecorsa l'Arduino "preme" anche
// STOP (doppia sicurezza oltre ai finecorsa della scheda del cancello).
#define FERMA_AL_FINECORSA  1

// ---- Ingressi -----------------------------------------------------------------
// Tutti gli ingressi usano la resistenza interna di "pull-up": ogni sensore
// e' un contatto collegato tra il pin e GND.
//   Contatto CHIUSO  -> pin a 0 V  -> "attivo"
//   Contatto APERTO o FILO TAGLIATO -> pin a 5 V -> "non attivo"
// In questo modo un filo interrotto non da' mai una falsa conferma
// (es. "tetto chiuso" o "montatura parcheggiata" quando non e' vero).
#define PIN_FC_APERTO               A0  // finecorsa NA: si chiude a tetto tutto aperto
#define PIN_FC_CHIUSO               A1  // finecorsa NA: si chiude a tetto tutto chiuso
#define PIN_MONTATURA_PARCHEGGIATA  A2  // contatto NA: si chiude con montatura in parcheggio
#define PIN_PIOGGIA                 A3  // contatto del rele' del sensore di pioggia
#define PIN_EMERGENZA               A4  // contatto NC del pulsante di emergenza
#define PIN_LED                     13  // LED sulla scheda Arduino

// Il pulsante di emergenza ha (oltre ai contatti che tolgono corrente al
// motore) un contatto ausiliario NC collegato qui: chiuso = tutto normale,
// aperto (pulsante premuto o filo interrotto) = EMERGENZA.
#define USA_INGRESSO_EMERGENZA  1

// Sensore di pioggia collegato anche al tetto (consigliato). Il contatto deve
// essere CHIUSO quando e' asciutto: cosi' un filo interrotto vale "pioggia".
#define USA_SENSORE_PIOGGIA     1

// ---- Tempi (in secondi) ------------------------------------------------------
// TEMPO_MAX_MOVIMENTO_S: misura con il cronometro quanto impiega il tetto ad
// aprirsi completamente e scrivi qui circa 1,3 volte quel tempo.
// Esempio: corsa di 60 s -> 80.
#define TEMPO_MAX_MOVIMENTO_S        80
// Entro quanti secondi il tetto deve lasciare il finecorsa di partenza.
#define TEMPO_MAX_DISTACCO_S         10
// Durante un movimento il driver interroga l'Arduino ogni secondo circa.
// Se tace per piu' di questi secondi -> STOP.
#define SILENZIO_MAX_IN_MOVIMENTO_S  15
// Da fermo il driver interroga ogni 3 secondi circa. Se tace per piu' di
// questi secondi l'Arduino considera il PC "muto".
#define SILENZIO_PC_MUTO_S          120

// ---- Chiusura autonoma per pioggia (DISATTIVATA di default) -------------------
// Se 1: quando il PC e' muto da SILENZIO_PC_MUTO_S secondi e piove da
// PIOGGIA_CONFERMA_S secondi, l'Arduino chiude il tetto DA SOLO, ma solo se
// la montatura risulta parcheggiata e tutti i controlli sono a posto.
// Attivala solo dopo il collaudo completo (vedi docs/07-collaudo.md).
#define CHIUSURA_AUTONOMA_PIOGGIA   0
#define PIOGGIA_CONFERMA_S         30

// Tempo di antirimbalzo dei contatti (millisecondi).
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

// ---- Stato del movimento -------------------------------------------------------
enum Movimento { FERMO, APERTURA, CHIUSURA };

// ---- Codici di errore (bloccano il tetto finche' non si riconnette Ekos) ------
enum Errore {
  ERR_NESSUNO = 0,
  ERR_TEMPO_SCADUTO,
  ERR_NON_PARTE,
  ERR_DIREZIONE,
  ERR_SENSORI,
  ERR_MONTATURA,
  ERR_EMERGENZA,
  ERR_COMUNICAZIONE
};

// Messaggi in minuscolo (vedi NOTA TECNICA in alto). Restano in memoria
// flash grazie a PROGMEM / F(), per non consumare la poca RAM dell'Uno.
const char msg0[] PROGMEM = "nessun errore";
const char msg1[] PROGMEM = "tempo massimo di movimento superato";
const char msg2[] PROGMEM = "il tetto non si e' staccato dal finecorsa";
const char msg3[] PROGMEM = "finecorsa opposto raggiunto: direzione errata";
const char msg4[] PROGMEM = "finecorsa incoerenti (aperto e chiuso insieme)";
const char msg5[] PROGMEM = "montatura uscita dal parcheggio";
const char msg6[] PROGMEM = "arresto di emergenza";
const char msg7[] PROGMEM = "comunicazione col pc persa durante il movimento";
const char* const MESSAGGI_ERRORE[] PROGMEM = { msg0, msg1, msg2, msg3, msg4, msg5, msg6, msg7 };

// ---- Lettura con antirimbalzo ----------------------------------------------------
struct Ingresso {
  uint8_t pin;
  bool stabile;          // valore valido: true = contatto chiuso (pin a GND)
  bool ultimoGrezzo;
  unsigned long tCambio;
};

Ingresso inAperto     = { PIN_FC_APERTO, false, false, 0 };
Ingresso inChiuso     = { PIN_FC_CHIUSO, false, false, 0 };
Ingresso inMontatura  = { PIN_MONTATURA_PARCHEGGIATA, false, false, 0 };
Ingresso inPioggia    = { PIN_PIOGGIA, false, false, 0 };
Ingresso inEmergenza  = { PIN_EMERGENZA, false, false, 0 };

// ---- Variabili globali -------------------------------------------------------------
Movimento     movimento = FERMO;
Errore        errore = ERR_NESSUNO;
bool          movimentoAutonomo = false;   // true se avviato dall'Arduino (pioggia)
bool          distaccoAvvenuto = false;
unsigned long tInizioMovimento = 0;
unsigned long tUltimoMessaggio = 0;
unsigned long tInizioPioggia = 0;
bool          pioggiaInCorso = false;
bool          tentativoAutonomoFatto = false;

// Buffer per i comandi in arrivo dal driver
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

void aggiornaIngresso(Ingresso &in) {
  bool grezzo = (digitalRead(in.pin) == LOW);
  unsigned long ora = millis();
  if (grezzo != in.ultimoGrezzo) {
    in.ultimoGrezzo = grezzo;
    in.tCambio = ora;
  } else if (grezzo != in.stabile && (ora - in.tCambio) >= ANTIRIMBALZO_MS) {
    in.stabile = grezzo;
  }
}

void aggiornaTuttiGliIngressi() {
  aggiornaIngresso(inAperto);
  aggiornaIngresso(inChiuso);
  aggiornaIngresso(inMontatura);
  aggiornaIngresso(inPioggia);
  aggiornaIngresso(inEmergenza);
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

// Il movimento e' consentito solo se TUTTE le condizioni sono vere.
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

// "Preme e rilascia" un tasto della scheda del cancello.
void impulso(uint8_t pinRele) {
  tuttiReleARiposo();                  // mai due tasti premuti insieme
  digitalWrite(pinRele, RELE_ON);
  delay(IMPULSO_MS);                   // breve: il watchdog (2 s) non scatta
  digitalWrite(pinRele, RELE_OFF);
  delay(PAUSA_DOPO_IMPULSO_MS);
  wdt_reset();
}

void premiStop() {
  impulso(PIN_RELE_STOP);
}

// Ferma il tetto e (se err != ERR_NESSUNO) registra un errore bloccante.
void arresto(Errore err) {
  premiStop();
  movimento = FERMO;
  movimentoAutonomo = false;
  if (err != ERR_NESSUNO) {
    errore = err;
  }
}

// =============================================================================
//  COMANDI DI MOVIMENTO
// =============================================================================

// Restituisce NULL se il movimento e' partito (o non serviva), altrimenti
// un messaggio (in flash) che spiega perche' e' stato rifiutato.
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

  // Se partiamo da un finecorsa, dobbiamo vederlo "liberarsi" entro poco.
  bool partenzaDaFinecorsa = (direzione == APERTURA) ? tettoChiuso() : tettoAperto();
  distaccoAvvenuto = !partenzaDaFinecorsa;

  impulso(direzione == APERTURA ? PIN_RELE_APRI : PIN_RELE_CHIUDI);
  movimento = direzione;
  movimentoAutonomo = autonomo;
  tInizioMovimento = millis();
  return NULL;
}

// Controlli continui durante il movimento (chiamata a ogni giro di loop).
void sorvegliaMovimento() {
  if (movimento == FERMO) return;

  unsigned long trascorso = millis() - tInizioMovimento;

  if (!emergenzaOk())           { arresto(ERR_EMERGENZA); return; }
  if (!montaturaParcheggiata()) { arresto(ERR_MONTATURA); return; }
  if (!finecorsaCoerenti())     { arresto(ERR_SENSORI);   return; }

  // Arrivato a destinazione?
  if ((movimento == APERTURA && tettoAperto()) ||
      (movimento == CHIUSURA && tettoChiuso())) {
#if FERMA_AL_FINECORSA
    premiStop();
#endif
    movimento = FERMO;
    movimentoAutonomo = false;
    return;
  }

  // Il tetto si e' staccato dal finecorsa di partenza?
  if (!distaccoAvvenuto) {
    bool ancoraSulFinecorsa = (movimento == APERTURA) ? tettoChiuso() : tettoAperto();
    if (!ancoraSulFinecorsa) {
      distaccoAvvenuto = true;
    } else if (trascorso > (unsigned long)TEMPO_MAX_DISTACCO_S * 1000UL) {
      arresto(ERR_NON_PARTE);
      return;
    }
  } else {
    // Gia' staccato: se tocca il finecorsa OPPOSTO a quello voluto,
    // il tetto sta andando nella direzione sbagliata.
    if ((movimento == APERTURA && tettoChiuso()) ||
        (movimento == CHIUSURA && tettoAperto())) {
      arresto(ERR_DIREZIONE);
      return;
    }
  }

  if (trascorso > (unsigned long)TEMPO_MAX_MOVIMENTO_S * 1000UL) {
    arresto(ERR_TEMPO_SCADUTO);
    return;
  }

  // Il PC deve continuare a parlare durante un movimento comandato da Ekos.
  if (!movimentoAutonomo &&
      (millis() - tUltimoMessaggio) > (unsigned long)SILENZIO_MAX_IN_MOVIMENTO_S * 1000UL) {
    arresto(ERR_COMUNICAZIONE);
    return;
  }
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
    tentativoAutonomoFatto = false;    // nuovo episodio di pioggia = nuovo tentativo
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

void rispondiNak(const __FlashStringHelper* messaggio) {
  Serial.print(F("(NAK:ERROR:"));
  Serial.print(messaggio);
  Serial.println(')');
}

void rispondiNakErrore() {
  char testo[60];
  strncpy_P(testo, (PGM_P)pgm_read_word(&(MESSAGGI_ERRORE[errore])), sizeof(testo) - 1);
  testo[sizeof(testo) - 1] = '\0';
  rispondi("NAK", "ERROR", testo);
}

// Interpreta un comando gia' ricevuto, nel formato COMANDO:OGGETTO:VALORE
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
    rispondi("ACK", "0", VERSIONE);
    return;
  }

  // ---- Lettura di uno stato ----
  if (strcmp(comando, "GET") == 0) {
    if      (strcmp(oggetto, "OPENED") == 0)   rispondiAck(oggetto, tettoAperto());
    else if (strcmp(oggetto, "CLOSED") == 0)   rispondiAck(oggetto, tettoChiuso());
    else if (strcmp(oggetto, "LOCKED") == 0)   rispondiAck(oggetto, movimento == FERMO && !movimentoConsentito());
    else if (strcmp(oggetto, "AUXSTATE") == 0) rispondiAck(oggetto, piove());
    else rispondiNak(F("richiesta sconosciuta"));
    return;
  }

  // ---- Comandi ----
  if (strcmp(comando, "SET") == 0) {
    bool acceso = (strcmp(valore, "ON") == 0);

    if (strcmp(oggetto, "OPEN") == 0 || strcmp(oggetto, "CLOSE") == 0) {
      if (!acceso) { rispondiAck(oggetto, false); return; }   // "rilascia": nulla da fare
      Movimento dir = (strcmp(oggetto, "OPEN") == 0) ? APERTURA : CHIUSURA;
      const __FlashStringHelper* rifiuto = avviaMovimento(dir, false);
      if (rifiuto == NULL) rispondiAck(oggetto, true);
      else                 rispondiNak(rifiuto);
      return;
    }
    if (strcmp(oggetto, "ABORT") == 0) {
      if (acceso) arresto(ERR_NESSUNO);   // STOP anche se il tetto e' stato mosso col telecomando
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

  // Comando "INFO" (non usato dal driver): utile al collaudo con
  // strumenti/prova_tetto.py per leggere l'errore bloccante corrente.
  if (strcmp(comando, "INF") == 0) {
    rispondiNakErrore();
    return;
  }

  rispondiNak(F("comando sconosciuto"));
}

// Legge i caratteri in arrivo senza mai bloccare il programma.
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
        dentroComando = false;             // comando troppo lungo: scartato
        rispondiNak(F("comando troppo lungo"));
      }
    }
  }
}

// =============================================================================
//  LED DI STATO (utile al collaudo)
//    acceso fisso         -> tetto in movimento
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
  // Prima di tutto: rele' a riposo. Si scrive il livello PRIMA di dichiarare
  // il pin come uscita, cosi' all'accensione nessun rele' "scatta".
  digitalWrite(PIN_RELE_APRI, RELE_OFF);
  digitalWrite(PIN_RELE_CHIUDI, RELE_OFF);
  digitalWrite(PIN_RELE_STOP, RELE_OFF);
  digitalWrite(PIN_RELE_RISERVA, RELE_OFF);
  pinMode(PIN_RELE_APRI, OUTPUT);
  pinMode(PIN_RELE_CHIUDI, OUTPUT);
  pinMode(PIN_RELE_STOP, OUTPUT);
  pinMode(PIN_RELE_RISERVA, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  inizializzaIngresso(inAperto);
  inizializzaIngresso(inChiuso);
  inizializzaIngresso(inMontatura);
  inizializzaIngresso(inPioggia);
  inizializzaIngresso(inEmergenza);

  Serial.begin(BAUD_RATE);
  tUltimoMessaggio = millis();

  wdt_enable(WDTO_2S);   // se il programma si blocca per 2 s, l'Arduino si riavvia
}

void loop() {
  wdt_reset();
  aggiornaTuttiGliIngressi();
  leggiSeriale();
  sorvegliaMovimento();
  controllaPioggiaAutonoma();
  aggiornaLed();
}

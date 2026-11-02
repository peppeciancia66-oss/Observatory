// =============================================================================
//  Prove automatiche di tetto.ino SENZA hardware.
//
//  Lo sketch viene compilato per il PC insieme a un "finto Arduino" e a un
//  piccolo simulatore del tetto (motore, finecorsa, montatura, pioggia,
//  emergenza). Un finto driver INDI invia gli stessi comandi del vero
//  driver "RollOff ino".
//
//  Uso:   ./esegui_prove.sh      (dalla cartella arduino/tetto/test)
// =============================================================================
#include "Arduino.h"
#include <cstdio>
#include <string>
#include <vector>

FintaSeriale Serial;
namespace finto {
  unsigned long ora_ms = 0;
  int livello[20];
  int uscita[20];
}

#include TETTO_INO     // lo sketch vero (percorso passato dal compilatore)

#include "simulatore_tetto.h"

// ---------------------------------------------------------------- finto driver
static void esegui(unsigned long ms) {
  for (unsigned long i = 0; i < ms; i++) { avanza(1); loop(); }
}

static std::string invia(const std::string& comando) {
  Serial.out.clear();
  for (char c : comando) Serial.in.push_back(c);
  for (int i = 0; i < 2000 && Serial.out.find("\r\n") == std::string::npos; i++) esegui(1);
  std::string r = Serial.out;
  if (r.size() >= 2) r.resize(r.size() - 2);
  Serial.out.clear();
  return r;
}

// Il driver vero interroga i quattro stati ogni 'periodo' ms.
static void interroga_per(unsigned long durata_ms, unsigned long periodo = 1000) {
  unsigned long fine = finto::ora_ms + durata_ms;
  while (finto::ora_ms < fine) {
    invia("(GET:OPENED:0)"); invia("(GET:CLOSED:0)");
    invia("(GET:LOCKED:0)"); invia("(GET:AUXSTATE:0)");
    esegui(periodo);
  }
}

// ---------------------------------------------------------------- prove
static int prove = 0, fallite = 0;
#define VERIFICA(cond) do { prove++; if (!(cond)) { fallite++; \
  printf("  FALLITA riga %d: %s\n", __LINE__, #cond); } } while (0)

static void ricomincia(double posizione) {
  sim = Simulatore();
  sim.posizione = posizione;
  finto::ora_ms = 0;
  Serial.in.clear(); Serial.out.clear();
  aggiorna_ingressi();
  setup();
  esegui(500);
  invia("(CON:0:0)");
  sim.impulsi.clear();
}

static bool contiene(const std::vector<std::string>& v, const std::string& s) {
  for (auto& x : v) if (x == s) return true;
  return false;
}

int main() {
  printf("Prove di tetto.ino\n");

  printf("- connessione e stati di base\n");
  ricomincia(0);
  VERIFICA(invia("(CON:0:0)") == "(ACK:0:" VERSIONE ")");
  VERIFICA(invia("(GET:CLOSED:0)") == "(ACK:CLOSED:ON)");
  VERIFICA(invia("(GET:OPENED:0)") == "(ACK:OPENED:OFF)");
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:OFF)");
  VERIFICA(invia("(GET:AUXSTATE:0)") == "(ACK:AUXSTATE:OFF)");
  VERIFICA(invia("(GET:PIPPO:0)").rfind("(NAK:ERROR:", 0) == 0);
  VERIFICA(invia("(XYZ)").rfind("(NAK:", 0) == 0);
  VERIFICA(sim.impulsi.empty());

  printf("- apertura completa\n");
  ricomincia(0);
  VERIFICA(invia("(SET:OPEN:ON)") == "(ACK:OPEN:ON)");
  VERIFICA(contiene(sim.impulsi, "APRI"));
  interroga_per(TEMPO_MAX_MOVIMENTO_S * 1000UL);
  VERIFICA(sim.posizione >= 99);
  VERIFICA(invia("(GET:OPENED:0)") == "(ACK:OPENED:ON)");
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:OFF)");
  VERIFICA(sim.impulsi.back() == "STOP");          // FERMA_AL_FINECORSA
  VERIFICA(sim.max_rele_attivi <= 1);               // mai due tasti insieme

  printf("- chiusura completa\n");
  sim.impulsi.clear();
  VERIFICA(invia("(SET:CLOSE:ON)") == "(ACK:CLOSE:ON)");
  interroga_per(TEMPO_MAX_MOVIMENTO_S * 1000UL);
  VERIFICA(sim.posizione <= 1);
  VERIFICA(invia("(GET:CLOSED:0)") == "(ACK:CLOSED:ON)");

  printf("- montatura non parcheggiata: nessun movimento (apertura e chiusura)\n");
  ricomincia(0);
  sim.montatura_parcheggiata = false;
  esegui(200);
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:ON)");
  std::string r = invia("(SET:OPEN:ON)");
  VERIFICA(r.rfind("(NAK:ERROR:", 0) == 0);
  VERIFICA(r.find("ON") == std::string::npos);      // il driver cerca "ON" nella risposta
  ricomincia(100);
  sim.montatura_parcheggiata = false;
  esegui(200);
  VERIFICA(invia("(SET:CLOSE:ON)").rfind("(NAK:", 0) == 0);
  VERIFICA(sim.impulsi.empty());

  printf("- montatura che esce dal parcheggio durante il movimento\n");
  ricomincia(0);
  invia("(SET:OPEN:ON)");
  interroga_per(5000);
  sim.montatura_parcheggiata = false;
  interroga_per(2000);
  VERIFICA(sim.direzione == 0 && sim.impulsi.back() == "STOP");
  sim.montatura_parcheggiata = true;
  interroga_per(2000);
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:ON)");   // errore bloccante
  VERIFICA(invia("(SET:OPEN:ON)").rfind("(NAK:", 0) == 0);
  invia("(CON:0:0)");                                       // riconnessione = riconoscimento
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:OFF)");

  printf("- tempo massimo superato\n");
  ricomincia(0);
  sim.velocita = 100.0 / (TEMPO_MAX_MOVIMENTO_S * 3.0);   // tetto lentissimo (es. attrito)
  invia("(SET:OPEN:ON)");
#if USA_SENSORE_MOVIMENTO
  // Con il sensore di movimento un tetto cosi' lento (un magnete ogni
  // ~13 s) viene fermato prima, come "fermo a meta'": e' il comportamento voluto.
  interroga_per((TEMPO_MAX_MOVIMENTO_S + 2) * 1000UL);
  VERIFICA(sim.direzione == 0 && sim.impulsi.back() == "STOP");
  VERIFICA(invia("(INF:ERRORE:0)").find("fermato a meta'") != std::string::npos);
#else
  interroga_per((TEMPO_MAX_MOVIMENTO_S - 2) * 1000UL);
  VERIFICA(sim.direzione == 1);
  interroga_per(4000);
  VERIFICA(sim.direzione == 0 && sim.impulsi.back() == "STOP");
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:ON)");
  VERIFICA(invia("(INF:ERRORE:0)").find("tempo massimo") != std::string::npos);
#endif

  printf("- motore che non parte\n");
  ricomincia(0);
  sim.motore_guasto = true;
  invia("(SET:OPEN:ON)");
  interroga_per((TEMPO_MAX_DISTACCO_S + 2) * 1000UL);
  VERIFICA(sim.impulsi.back() == "STOP");
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:ON)");
  VERIFICA(invia("(INF:ERRORE:0)").find("partito") != std::string::npos);

  printf("- direzione invertita (partenza da tetto a meta')\n");
  ricomincia(50);
  sim.invertito = true;
  invia("(SET:OPEN:ON)");
  interroga_per(TEMPO_MAX_MOVIMENTO_S * 1000UL);
  VERIFICA(sim.impulsi.back() == "STOP");
  VERIFICA(invia("(INF:ERRORE:0)").find("direzione") != std::string::npos);

  printf("- PC che smette di rispondere durante il movimento\n");
  ricomincia(0);
  invia("(SET:OPEN:ON)");
  interroga_per(3000);
  esegui((SILENZIO_MAX_IN_MOVIMENTO_S - 1) * 1000UL);       // nessuna interrogazione
  VERIFICA(sim.direzione == 1);
  esegui(3000);
  VERIFICA(sim.direzione == 0 && sim.impulsi.back() == "STOP");
  VERIFICA(sim.posizione < 99);
  invia("(CON:0:0)");
  VERIFICA(invia("(INF:ERRORE:0)").find("nessun errore") != std::string::npos);

  printf("- finecorsa incoerenti\n");
  ricomincia(0);
  sim.forza_fc_aperto = 1;                                  // aperto E chiuso insieme
  esegui(200);
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:ON)");
  VERIFICA(invia("(SET:OPEN:ON)").rfind("(NAK:", 0) == 0);
  VERIFICA(sim.impulsi.empty());

  printf("- finecorsa che si guasta durante il movimento\n");
  ricomincia(0);
  invia("(SET:OPEN:ON)");
  interroga_per(10000);
  sim.forza_fc_aperto = 1; sim.forza_fc_chiuso = 1;
  interroga_per(1000);
  VERIFICA(sim.impulsi.back() == "STOP");
  VERIFICA(invia("(INF:ERRORE:0)").find("incoerenti") != std::string::npos);

  printf("- arresto di emergenza\n");
  ricomincia(0);
  invia("(SET:OPEN:ON)");
  interroga_per(5000);
  sim.emergenza = true;
  interroga_per(1000);
  VERIFICA(sim.direzione == 0);
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:ON)");
  invia("(CON:0:0)");
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:ON)");   // finche' il pulsante e' premuto
  sim.emergenza = false;
  esegui(200);
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:OFF)");

  printf("- ABORT dall'utente: si ferma senza errore bloccante\n");
  ricomincia(0);
  invia("(SET:OPEN:ON)");
  interroga_per(10000);
  VERIFICA(invia("(SET:ABORT:ON)") == "(ACK:ABORT:ON)");
  VERIFICA(sim.direzione == 0 && sim.impulsi.back() == "STOP");
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:OFF)");
  VERIFICA(invia("(SET:CLOSE:ON)") == "(ACK:CLOSE:ON)");
  interroga_per(TEMPO_MAX_MOVIMENTO_S * 1000UL);
  VERIFICA(sim.posizione <= 1);

  printf("- pioggia letta dal tetto (luce Auxiliary) e chiusura autonoma disattivata\n");
  ricomincia(100);
  sim.pioggia = true;
  esegui(200);
  VERIFICA(invia("(GET:AUXSTATE:0)") == "(ACK:AUXSTATE:ON)");
  esegui((SILENZIO_PC_MUTO_S + PIOGGIA_CONFERMA_S + 30) * 1000UL);  // PC muto
#if CHIUSURA_AUTONOMA_PIOGGIA
  printf("- (versione di prova con chiusura autonoma ATTIVA)\n");
  VERIFICA(sim.posizione <= 1);
  VERIFICA(contiene(sim.impulsi, "CHIUDI"));
  // con montatura non parcheggiata NON deve chiudere
  ricomincia(100);
  sim.montatura_parcheggiata = false;
  sim.pioggia = true;
  esegui((SILENZIO_PC_MUTO_S + PIOGGIA_CONFERMA_S + 30) * 1000UL);
  VERIFICA(sim.posizione >= 99 && sim.impulsi.empty());
  // con il PC che risponde NON deve chiudere da solo
  ricomincia(100);
  sim.pioggia = true;
  interroga_per((SILENZIO_PC_MUTO_S + PIOGGIA_CONFERMA_S + 30) * 1000UL, 3000);
  VERIFICA(sim.posizione >= 99 && sim.impulsi.empty());
#else
  VERIFICA(sim.impulsi.empty() && sim.posizione >= 99);
#endif

  printf("- stato in parole (in apertura / aperto / fermo a meta')\n");
  ricomincia(0);
  VERIFICA(invia("(INF:STATO:0)") == "(ACK:STATO:chiuso)");
  invia("(SET:OPEN:ON)");
  interroga_per(3000);
  VERIFICA(invia("(INF:STATO:0)") == "(ACK:STATO:in apertura)");
  invia("(SET:ABORT:ON)");
  interroga_per(1000);
  VERIFICA(invia("(INF:STATO:0)") == "(ACK:STATO:fermo a meta')");
  invia("(SET:OPEN:ON)");
  interroga_per(TEMPO_MAX_MOVIMENTO_S * 1000UL);
  VERIFICA(invia("(INF:STATO:0)") == "(ACK:STATO:aperto)");

  printf("- movimento NON comandato con montatura fuori parcheggio -> STOP (solo avviso)\n");
  ricomincia(100);
  sim.montatura_parcheggiata = false;          // si sta osservando
  esegui(TOLLERANZA_DOPO_MOVIMENTO_MS + 500);
  sim.direzione = -1;                          // es. richiusura automatica della scheda
  interroga_per(3000);
  VERIFICA(sim.direzione == 0 && contiene(sim.impulsi, "STOP"));
  VERIFICA(sim.posizione > 90);
  VERIFICA(invia("(INF:ERRORE:0)").find("non comandato") != std::string::npos);
  sim.montatura_parcheggiata = true;           // fine sessione: parcheggio
  esegui(200);
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:OFF)");   // NON bloccato
  VERIFICA(invia("(SET:CLOSE:ON)") == "(ACK:CLOSE:ON)");
  interroga_per(TEMPO_MAX_MOVIMENTO_S * 1000UL);
  VERIFICA(sim.posizione <= 1);

  printf("- movimento non comandato con montatura parcheggiata (telecomando): nessun intervento\n");
  ricomincia(0);
  esegui(TOLLERANZA_DOPO_MOVIMENTO_MS + 500);
  sim.direzione = 1;
  interroga_per(2000);
  // Senza sensore di movimento lo si sa solo per pochi secondi dopo che il
  // tetto ha lasciato il finecorsa; con il sensore, per tutto il movimento.
  VERIFICA(invia("(INF:STATO:0)") == "(ACK:STATO:movimento non comandato)");
  interroga_per(3000);
  VERIFICA(sim.direzione == 1 && sim.impulsi.empty());

  printf("- vibrazione breve del finecorsa (< antirimbalzo) ignorata\n");
  ricomincia(100);
  sim.montatura_parcheggiata = false;
  esegui(TOLLERANZA_DOPO_MOVIMENTO_MS + 500);
  sim.forza_fc_aperto = 0; esegui(20); sim.forza_fc_aperto = -1;
  interroga_per(2000);
  VERIFICA(sim.impulsi.empty());

#if USA_SENSORE_MOVIMENTO
  printf("- (sensore di movimento) tetto che si ferma a meta' durante il movimento\n");
  ricomincia(0);
  invia("(SET:OPEN:ON)");
  interroga_per(10000);
  VERIFICA(sim.direzione == 1);
  sim.direzione = 0;                            // es. ostacolo: la scheda ferma il motore
  sim.motore_guasto = true;
  interroga_per(FINESTRA_MOVIMENTO_MS + 2000);
  VERIFICA(invia("(INF:ERRORE:0)").find("fermato a meta'") != std::string::npos);
  VERIFICA(invia("(GET:LOCKED:0)") == "(ACK:LOCKED:ON)");

  printf("- (sensore di movimento) partenza da meta' con motore che non parte\n");
  ricomincia(50);
  sim.motore_guasto = true;
  invia("(SET:CLOSE:ON)");
  interroga_per((TEMPO_MAX_DISTACCO_S + 2) * 1000UL);
  VERIFICA(sim.impulsi.back() == "STOP");
  VERIFICA(invia("(INF:ERRORE:0)").find("partito") != std::string::npos);

  printf("- (sensore di movimento) movimento non comandato rilevato anche a meta' corsa\n");
  ricomincia(50);
  sim.montatura_parcheggiata = false;
  esegui(TOLLERANZA_DOPO_MOVIMENTO_MS + 500);
  sim.direzione = 1;
  interroga_per(6000);
  VERIFICA(sim.direzione == 0 && contiene(sim.impulsi, "STOP"));
#endif

  printf("- comando troppo lungo o spezzato non blocca nulla\n");
  ricomincia(0);
  VERIFICA(invia("(" + std::string(80, 'A') + ")").rfind("(NAK:", 0) == 0);
  VERIFICA(invia("spazzatura(GET:CLOSED:0)") == "(ACK:CLOSED:ON)");

  printf("\n%d verifiche, %d fallite\n", prove, fallite);
  return fallite == 0 ? 0 : 1;
}

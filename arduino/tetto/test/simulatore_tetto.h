// =============================================================================
//  simulatore_tetto.h - tetto, motore e sensori simulati, condivisi da
//  prova_tetto.cpp (prove automatiche) e tetto_virtuale.cpp (Arduino virtuale).
//  Va incluso DOPO lo sketch (usa i suoi #define).
// =============================================================================
#pragma once
#include <string>
#include <vector>

// ---------------------------------------------------------------- simulatore
struct Simulatore {
  double posizione = 0;          // 0 = tutto chiuso, 100 = tutto aperto
  int direzione = 0;             // +1 apre, -1 chiude, 0 fermo
  // corsa completa in un tempo pari al 60% del tempo massimo configurato
  double velocita = 100.0 / (TEMPO_MAX_MOVIMENTO_S * 0.6);
  bool motore_guasto = false;    // la scheda non fa muovere il motore
  bool invertito = false;        // apri e chiudi scambiati sulla scheda
  bool montatura_parcheggiata = true;
  bool pioggia = false;
  bool emergenza = false;
  int forza_fc_aperto = -1;      // -1 normale, 0/1 forzato (finecorsa guasto)
  int forza_fc_chiuso = -1;
  std::vector<std::string> impulsi;
  int rele_attivi = 0, max_rele_attivi = 0;
} sim;

static void aggiorna_ingressi() {
  bool aperto = sim.posizione >= 99.0;
  bool chiuso = sim.posizione <= 1.0;
  if (sim.forza_fc_aperto >= 0) aperto = sim.forza_fc_aperto;
  if (sim.forza_fc_chiuso >= 0) chiuso = sim.forza_fc_chiuso;
  // contatto chiuso = pin a GND = LOW
  finto::livello[PIN_FC_APERTO] = aperto ? LOW : HIGH;
  finto::livello[PIN_FC_CHIUSO] = chiuso ? LOW : HIGH;
  finto::livello[PIN_MONTATURA_PARCHEGGIATA] = sim.montatura_parcheggiata ? LOW : HIGH;
  finto::livello[PIN_PIOGGIA] = sim.pioggia ? HIGH : LOW;      // chiuso = asciutto
  finto::livello[PIN_EMERGENZA] = sim.emergenza ? HIGH : LOW;  // NC chiuso = ok
  // sensore di movimento: un magnete ogni 10 unita' di corsa, largo 1,5 unita'
  double resto = sim.posizione - 10.0 * (int)(sim.posizione / 10.0);
  finto::livello[PIN_SENSORE_MOVIMENTO] = (resto > 4.0 && resto < 5.5) ? LOW : HIGH;
}

static void avanza(unsigned long ms) {
  for (unsigned long i = 0; i < ms; i++) {
    finto::ora_ms++;
    sim.posizione += sim.direzione * sim.velocita / 1000.0;
    if (sim.posizione >= 100) { sim.posizione = 100; sim.direzione = 0; }  // finecorsa della scheda
    if (sim.posizione <= 0)   { sim.posizione = 0;   sim.direzione = 0; }
    aggiorna_ingressi();
  }
}

void finto::durante_attesa(unsigned long ms) { avanza(ms); }

void finto::quando_scritto(uint8_t pin, int valore) {
  if (pin != PIN_RELE_APRI && pin != PIN_RELE_CHIUDI && pin != PIN_RELE_STOP && pin != PIN_RELE_RISERVA) return;
  int attivi = 0;
  for (int p : {PIN_RELE_APRI, PIN_RELE_CHIUDI, PIN_RELE_STOP, PIN_RELE_RISERVA})
    if (finto::uscita[p] == RELE_ON) attivi++;
  if (attivi > sim.max_rele_attivi) sim.max_rele_attivi = attivi;
  if (valore != RELE_ON) return;
  const char* nome = pin == PIN_RELE_APRI ? "APRI" : pin == PIN_RELE_CHIUDI ? "CHIUDI" :
                     pin == PIN_RELE_STOP ? "STOP" : "RISERVA";
  sim.impulsi.push_back(nome);
  if (pin == PIN_RELE_STOP) sim.direzione = 0;
  else if (!sim.motore_guasto && !sim.emergenza) {
    int d = (pin == PIN_RELE_APRI) ? 1 : -1;
    sim.direzione = sim.invertito ? -d : d;
  }
}


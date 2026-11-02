// =============================================================================
//  tetto_virtuale.cpp - "Arduino virtuale": lo sketch tetto.ino vero, eseguito
//  sul PC in tempo reale, con il tetto simulato, collegato a una finta porta
//  seriale (pseudo-terminale). Serve a provare strumenti/prova_tetto.py (o
//  perfino Ekos) SENZA hardware.
//
//  Uso:  bash esegui_tetto_virtuale.sh
//        poi, in un altro terminale:  python3 strumenti/prova_tetto.py /tmp/tetto-virtuale
//
//  Comandi da tastiera in questa finestra (per simulare il mondo esterno):
//     m = montatura parcheggiata si'/no      e = emergenza premuta si'/no
//     p = pioggia si'/no                     g = motore guasto si'/no
//     t = telecomando: muove il tetto senza passare dall'Arduino
//     s = mostra lo stato del tetto simulato  q = esci
// =============================================================================
#include "Arduino.h"
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include <chrono>
#include <thread>

FintaSeriale Serial;
namespace finto {
  unsigned long ora_ms = 0;
  int livello[20];
  int uscita[20];
}

#include TETTO_INO
#include "simulatore_tetto.h"

int main(int argc, char** argv) {
  const char* collegamento = argc > 1 ? argv[1] : "/tmp/tetto-virtuale";
  int master = posix_openpt(O_RDWR | O_NOCTTY);
  if (master < 0 || grantpt(master) || unlockpt(master)) { perror("pty"); return 1; }
  const char* slave = ptsname(master);
  // la parte "slave" in modalita' grezza, come una vera porta seriale
  int sfd = open(slave, O_RDWR | O_NOCTTY);
  struct termios t; tcgetattr(sfd, &t); cfmakeraw(&t); tcsetattr(sfd, TCSANOW, &t);
  unlink(collegamento);
  if (symlink(slave, collegamento)) { perror("symlink"); return 1; }
  fcntl(master, F_SETFL, O_NONBLOCK);
  fcntl(0, F_SETFL, O_NONBLOCK);

  sim.posizione = 0;
  aggiorna_ingressi();
  setup();
  printf("Arduino virtuale pronto su %s (tetto chiuso, montatura parcheggiata).\n", collegamento);
  printf("Tasti: m e p g t s q + Invio\n");
  fflush(stdout);

  auto prossimo = std::chrono::steady_clock::now();
  bool attivo = true;
  while (attivo) {
    char buf[256];
    ssize_t n = read(master, buf, sizeof buf);
    for (ssize_t i = 0; i < n; i++) Serial.in.push_back(buf[i]);

    char c;
    while (read(0, &c, 1) == 1) {
      switch (c) {
        case 'm': sim.montatura_parcheggiata = !sim.montatura_parcheggiata;
                  printf("montatura parcheggiata: %s\n", sim.montatura_parcheggiata ? "si" : "no"); break;
        case 'e': sim.emergenza = !sim.emergenza; printf("emergenza: %s\n", sim.emergenza ? "PREMUTA" : "no"); break;
        case 'p': sim.pioggia = !sim.pioggia; printf("pioggia: %s\n", sim.pioggia ? "si" : "no"); break;
        case 'g': sim.motore_guasto = !sim.motore_guasto; printf("motore guasto: %s\n", sim.motore_guasto ? "si" : "no"); break;
        case 't': sim.direzione = sim.posizione > 50 ? -1 : 1; printf("telecomando: il tetto si muove da solo\n"); break;
        case 's': printf("posizione %.0f%%, motore %s\n", sim.posizione,
                         sim.direzione > 0 ? "apre" : sim.direzione < 0 ? "chiude" : "fermo"); break;
        case 'q': attivo = false; break;
      }
      fflush(stdout);
    }

    avanza(1);
    loop();
    if (!Serial.out.empty()) {
      ssize_t w = write(master, Serial.out.data(), Serial.out.size());
      (void)w;
      Serial.out.clear();
    }
    prossimo += std::chrono::milliseconds(1);
    std::this_thread::sleep_until(prossimo);
  }
  unlink(collegamento);
  return 0;
}

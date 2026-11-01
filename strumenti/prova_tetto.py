#!/usr/bin/env python3
"""Prova del controllore del tetto SENZA Ekos (per il collaudo).

Parla con l'Arduino del tetto usando gli stessi comandi del driver INDI e
mostra in italiano lo stato dei sensori. Mentre il programma e' aperto
interroga l'Arduino ogni secondo, proprio come fa il driver: se lo chiudi
durante un movimento, l'Arduino ferma il tetto dopo pochi secondi (ed e'
giusto cosi': e' la protezione "PC muto").

IMPORTANTE: chiudi Ekos (o disconnetti il tetto in Ekos) prima di usarlo:
la porta USB puo' essere usata da un solo programma alla volta.

Uso:
    python3 prova_tetto.py                      (porta /dev/osservatorio-tetto)
    python3 prova_tetto.py /dev/ttyACM0         (altra porta)
"""

import sys
import threading
import time

try:
    import serial
except ImportError:
    print("Manca il pacchetto python3-serial. Installa con:\n  sudo apt install python3-serial")
    sys.exit(1)

PORTA_PREDEFINITA = "/dev/osservatorio-tetto"
VELOCITA = 38400

NOMI = {
    "OPENED": "Finecorsa TETTO APERTO",
    "CLOSED": "Finecorsa TETTO CHIUSO",
    "LOCKED": "Movimento BLOCCATO",
    "AUXSTATE": "Pioggia (sensore del tetto)",
}


class Tetto:
    def __init__(self, porta):
        self.s = serial.Serial(porta, VELOCITA, timeout=3)
        self.lock = threading.Lock()
        print("Attendo il riavvio dell'Arduino (2 secondi)...")
        time.sleep(2.5)
        self.s.reset_input_buffer()

    def invia(self, comando):
        with self.lock:
            self.s.reset_input_buffer()
            self.s.write(comando.encode("ascii"))
            risposta = self.s.read_until(b")").decode("ascii", errors="replace").strip()
            return risposta or "(nessuna risposta!)"

    def stato(self):
        valori = {}
        for chiave in NOMI:
            r = self.invia(f"(GET:{chiave}:0)")
            valori[chiave] = "ON" if r.endswith(":ON)") else ("OFF" if r.endswith(":OFF)") else r)
        return valori


def stampa_stato(valori):
    for chiave, nome in NOMI.items():
        v = valori[chiave]
        testo = {"ON": "SI", "OFF": "no"}.get(v, f"?? {v}")
        print(f"   {nome:<30} {testo}")


def main():
    porta = sys.argv[1] if len(sys.argv) > 1 else PORTA_PREDEFINITA
    try:
        tetto = Tetto(porta)
    except serial.SerialException as e:
        print(f"Non riesco ad aprire {porta}: {e}")
        print("Controlla il cavo USB, che Ekos sia chiuso e il nome della porta.")
        return 1

    print("Connessione:", tetto.invia("(CON:0:0)"))
    attivo = True
    ultimo = {}

    def sorveglia():
        nonlocal ultimo
        while attivo:
            try:
                valori = tetto.stato()
            except Exception as e:
                print(f"\n!! Errore di comunicazione: {e}")
                return
            if valori != ultimo:
                print(f"\n[{time.strftime('%H:%M:%S')}] Cambio di stato:")
                stampa_stato(valori)
                if valori.get("LOCKED") == "ON":
                    print("   Motivo del blocco:", tetto.invia("(INF:0:0)"))
                print("Scelta: ", end="", flush=True)
                ultimo = valori
            time.sleep(1)

    threading.Thread(target=sorveglia, daemon=True).start()

    menu = """
  1 = mostra lo stato        4 = STOP
  2 = APRI                   5 = perche' e' bloccato?
  3 = CHIUDI                 6 = azzera errore (come riconnettere Ekos)
  0 = esci
"""
    try:
        while True:
            print(menu)
            scelta = input("Scelta: ").strip()
            if scelta == "1":
                stampa_stato(tetto.stato())
            elif scelta == "2":
                print("Risposta:", tetto.invia("(SET:OPEN:ON)"))
            elif scelta == "3":
                print("Risposta:", tetto.invia("(SET:CLOSE:ON)"))
            elif scelta == "4":
                print("Risposta:", tetto.invia("(SET:ABORT:ON)"))
            elif scelta == "5":
                print("Risposta:", tetto.invia("(INF:0:0)"))
            elif scelta == "6":
                print("Risposta:", tetto.invia("(CON:0:0)"))
            elif scelta == "0":
                break
    except (KeyboardInterrupt, EOFError):
        print()
    finally:
        attivo = False
        print("Invio STOP per sicurezza e chiudo.")
        try:
            tetto.invia("(SET:ABORT:ON)")
        except Exception:
            pass
    return 0


if __name__ == "__main__":
    sys.exit(main())

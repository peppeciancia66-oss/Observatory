#!/usr/bin/env python3
"""Script chiamato dal driver INDI "Weather Safety Proxy" (ogni 60 s circa).

Legge il file di stato scritto dal programma meteo e stampa UNA riga JSON
nel formato che il driver si aspetta:

    {"roof_status": {"open_ok": 1, "reasons": "condizioni buone"}}

    open_ok = 1 -> condizioni SICURE
    open_ok = 0 -> condizioni NON SICURE (con il motivo in "reasons")

REGOLA FONDAMENTALE: nel dubbio si risponde NON SICURO. Quindi open_ok = 0 se:
  - il file di stato non esiste o e' illeggibile;
  - il file non viene aggiornato da troppo tempo (programma meteo bloccato);
  - il programma e' in modalita' taratura;
  - succede qualunque errore imprevisto in questo script.

Lo script stampa SEMPRE un JSON valido: il driver, se ricevesse testo non
valido, andrebbe in errore.

Non usa librerie esterne, solo Python standard.
"""

import configparser
import json
import os
import sys
import time

CONFIG_PREDEFINITA = "~/.config/osservatorio/meteo.ini"
CARTELLA_PREDEFINITA = "~/osservatorio-meteo"
STATO_VECCHIO_PREDEFINITO_S = 90.0


def risposta(sicuro, motivo):
    return json.dumps({"roof_status": {"open_ok": 1 if sicuro else 0,
                                       "reasons": str(motivo)[:400]}},
                      ensure_ascii=True)


def leggi_impostazioni():
    """Legge da meteo.ini la cartella e l'eta' massima dello stato."""
    percorso = os.path.expanduser(os.environ.get("OSSERVATORIO_METEO_INI", CONFIG_PREDEFINITA))
    cartella = CARTELLA_PREDEFINITA
    eta_massima = STATO_VECCHIO_PREDEFINITO_S
    parser = configparser.ConfigParser(inline_comment_prefixes=(";", "#"))
    if parser.read(percorso, encoding="utf-8"):
        cartella = parser.get("registro", "cartella", fallback=cartella).strip()
        try:
            eta_massima = float(parser.get("sicurezza", "stato_vecchio_dopo_s",
                                           fallback=str(eta_massima)))
        except ValueError:
            pass  # valore scritto male: si tiene quello predefinito
    return os.path.join(os.path.expanduser(cartella), "stato_meteo.json"), eta_massima


def valuta(ora=None):
    ora = time.time() if ora is None else ora
    file_stato, eta_massima = leggi_impostazioni()
    try:
        with open(file_stato, encoding="utf-8") as f:
            stato = json.load(f)
    except FileNotFoundError:
        return risposta(False, "programma meteo mai avviato (file di stato assente)")
    except (OSError, ValueError) as e:
        return risposta(False, f"file di stato illeggibile: {e}")

    if not isinstance(stato, dict):
        return risposta(False, "file di stato non valido")
    aggiornato = stato.get("aggiornato")
    if not isinstance(aggiornato, (int, float)) or isinstance(aggiornato, bool):
        return risposta(False, "file di stato senza data di aggiornamento")
    eta = ora - aggiornato
    if eta > eta_massima:
        return risposta(False, f"programma meteo fermo: ultimo aggiornamento {eta:.0f} s fa")
    if eta < -60:
        return risposta(False, "orologio del PC incoerente con il file di stato")

    motivi = stato.get("motivi") or ""
    if stato.get("modalita") != "normale":
        return risposta(False, motivi or "modalita' taratura")
    if stato.get("sicuro") is True:
        return risposta(True, motivi or "condizioni buone")
    return risposta(False, motivi or "condizioni non sicure")


def main():
    try:
        testo = valuta()
    except Exception as e:  # qualunque imprevisto -> NON SICURO, mai un crash
        testo = risposta(False, f"errore interno dello script: {type(e).__name__}")
    sys.stdout.write(testo + "\n")
    sys.stdout.flush()
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception:
        sys.stdout.write('{"roof_status": {"open_ok": 0, "reasons": "errore grave dello script"}}\n')
        sys.exit(0)

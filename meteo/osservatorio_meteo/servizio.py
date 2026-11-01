"""Programma principale della stazione meteo.

Gira sempre in sottofondo (servizio "osservatorio-meteo") e:
  1. legge le misure dall'Arduino meteo (o dal simulatore);
  2. calcola se le condizioni sono sicure (modulo valutazione);
  3. scrive lo stato in un piccolo file (stato_meteo.json) che lo script
     weather_status.py passa a Ekos tramite il driver "Weather Safety Proxy";
  4. registra i dati in file CSV giornalieri (per la taratura);
  5. avvisa con notifiche sul desktop in caso di guasti.
"""

from __future__ import annotations

import argparse
import csv
import json
import logging
import logging.handlers
import os
import signal
import sys
import time
from datetime import datetime
from pathlib import Path

from . import configurazione
from .avvisi import Avvisatore, GestoreAvvisi
from .configurazione import Configurazione, ErroreConfigurazione
from .sorgenti import SorgenteSeriale, SorgenteSimulata
from .valutazione import Esito, RigaNonValida, Valutatore, interpreta_riga

log = logging.getLogger("meteo")

VERSIONE_STATO = 1
COLONNE_CSV = [
    "data_ora", "modalita", "t_aria", "t_cielo", "differenza",
    "vento_medio_kmh", "raffica_kmh", "pioggia",
    "condizioni_buone", "sicuro_calcolato", "sicuro_comunicato", "motivi",
]


def _arrotonda(valore, cifre=2):
    return None if valore is None else round(valore, cifre)


def scrivi_stato(percorso: Path, esito: Esito | None, cfg: Configurazione,
                 motivo_forzato: str | None = None) -> None:
    """Scrive il file di stato in modo "atomico" (mai mezzo scritto)."""
    if esito is None or motivo_forzato:
        sicuro = False
        motivi = motivo_forzato or "stato sconosciuto"
        misure = {}
        guasti, avvisi = [], []
        sicuro_calcolato = False
    else:
        sicuro = esito.sicuro_comunicato
        sicuro_calcolato = esito.sicuro_calcolato
        motivi = esito.testo_motivi()
        if cfg.in_taratura:
            esito_simulato = "SICURO" if esito.sicuro_calcolato else "NON SICURO"
            motivi = (f"MODALITA' TARATURA: a Ekos si comunica sempre NON SICURO. "
                      f"Con le soglie attuali sarebbe {esito_simulato} ({motivi})")
        guasti, avvisi = esito.guasti, esito.avvisi
        misure = {
            "t_aria": _arrotonda(esito.aria),
            "t_cielo": _arrotonda(esito.cielo),
            "differenza": _arrotonda(esito.differenza),
            "vento_medio_kmh": _arrotonda(esito.vento_medio_kmh, 1),
            "raffica_kmh": _arrotonda(esito.raffica_kmh, 1),
            "pioggia": esito.pioggia,
        }
    ora = time.time()
    dati = {
        "versione": VERSIONE_STATO,
        "aggiornato": ora,
        "aggiornato_leggibile": datetime.fromtimestamp(ora).isoformat(timespec="seconds"),
        "modalita": cfg.modalita,
        "sicuro": bool(sicuro),
        "sicuro_calcolato": bool(sicuro_calcolato),
        "motivi": motivi,
        "guasti": guasti,
        "avvisi": avvisi,
        "misure": misure,
    }
    percorso.parent.mkdir(parents=True, exist_ok=True)
    temporaneo = percorso.with_suffix(".tmp")
    with open(temporaneo, "w", encoding="utf-8") as f:
        json.dump(dati, f, ensure_ascii=False, indent=1)
        f.flush()
        os.fsync(f.fileno())
    os.replace(temporaneo, percorso)


def scrivi_registro(cartella: Path, esito: Esito, cfg: Configurazione) -> None:
    adesso = datetime.now()
    percorso = cartella / "registro" / f"{adesso:%Y-%m-%d}.csv"
    percorso.parent.mkdir(parents=True, exist_ok=True)
    nuovo = not percorso.exists()
    with open(percorso, "a", newline="", encoding="utf-8") as f:
        w = csv.writer(f, delimiter=";")   # ";" si apre bene in LibreOffice in italiano
        if nuovo:
            w.writerow(COLONNE_CSV)

        def num(v, cifre=2):
            # virgola decimale, come si aspetta LibreOffice in italiano
            return "" if v is None else f"{v:.{cifre}f}".replace(".", ",")

        def sino(v):
            return "" if v is None else ("si" if v else "no")

        w.writerow([
            adesso.isoformat(sep=" ", timespec="seconds"), cfg.modalita,
            num(esito.aria), num(esito.cielo), num(esito.differenza),
            num(esito.vento_medio_kmh, 1), num(esito.raffica_kmh, 1), sino(esito.pioggia),
            sino(esito.condizioni_buone), sino(esito.sicuro_calcolato),
            sino(esito.sicuro_comunicato), esito.testo_motivi(),
        ])


def configura_log(cartella: Path, verboso: bool) -> None:
    cartella.mkdir(parents=True, exist_ok=True)
    formato = logging.Formatter("%(asctime)s %(levelname)s %(message)s", "%Y-%m-%d %H:%M:%S")
    su_file = logging.handlers.RotatingFileHandler(
        cartella / "meteo.log", maxBytes=2_000_000, backupCount=5, encoding="utf-8")
    su_file.setFormatter(formato)
    a_video = logging.StreamHandler(sys.stderr)
    a_video.setFormatter(formato)
    log.handlers[:] = [su_file, a_video]
    log.setLevel(logging.DEBUG if verboso else logging.INFO)


class Servizio:
    INTERVALLO_VALUTAZIONE_S = 2.0
    INTERVALLO_CONTROLLO_CONFIG_S = 5.0

    def __init__(self, cfg: Configurazione, mostra: bool = False):
        self.cfg = cfg
        self.mostra = mostra
        self.attivo = True
        self.valutatore = Valutatore(time.monotonic())
        self.avvisatore = Avvisatore(cfg.notifiche_desktop)
        self.avvisi = GestoreAvvisi(self.avvisatore)
        self.mtime_config = self._mtime()
        self.righe_scartate = 0
        if cfg.porta.lower() == "simulazione":
            log.warning("Porta = simulazione: i dati NON vengono dai sensori veri!")
            self.sorgente = SorgenteSimulata(lambda: self.cfg)
        else:
            self.sorgente = SorgenteSeriale(cfg.porta, cfg.velocita)

    def _mtime(self) -> float:
        try:
            return self.cfg.percorso.stat().st_mtime
        except OSError:
            return 0.0

    def _ricarica_config_se_cambiata(self) -> None:
        mtime = self._mtime()
        if mtime == self.mtime_config:
            return
        self.mtime_config = mtime
        try:
            nuova = configurazione.carica(self.cfg.percorso)
        except ErroreConfigurazione as e:
            self.avvisatore.avvisa("meteo.ini contiene un errore",
                                   f"{e} Continuo con la configurazione precedente.",
                                   urgente=True)
            return
        if (nuova.porta, nuova.velocita, nuova.cartella_registro) != \
                (self.cfg.porta, self.cfg.velocita, self.cfg.cartella_registro):
            log.warning("Porta, velocita' o cartella cambiate: avranno effetto solo dopo "
                        "il riavvio (systemctl --user restart osservatorio-meteo)")
            nuova.porta, nuova.velocita = self.cfg.porta, self.cfg.velocita
            nuova.cartella_registro = self.cfg.cartella_registro
        if nuova.modalita != self.cfg.modalita:
            self.avvisatore.avvisa("Stazione meteo", f"Modalita' cambiata: {nuova.modalita}")
        self.cfg = nuova
        self.avvisatore.notifiche_desktop = nuova.notifiche_desktop
        log.info("Configurazione ricaricata da %s", nuova.percorso)

    def ferma(self, *_args) -> None:
        self.attivo = False

    def esegui(self) -> None:
        log.info("Stazione meteo avviata. Modalita': %s. Porta: %s",
                 self.cfg.modalita, self.cfg.porta)
        prossima_valutazione = 0.0
        prossimo_registro = 0.0
        prossimo_controllo_config = 0.0
        try:
            while self.attivo:
                riga = self.sorgente.leggi_riga()
                adesso = time.monotonic()
                if riga:
                    try:
                        misura = interpreta_riga(riga)
                        self.valutatore.aggiungi(misura, adesso, self.cfg)
                        log.debug("Ricevuto: %s", riga)
                    except RigaNonValida as e:
                        self.righe_scartate += 1
                        # i primi caratteri dopo l'accensione dell'Arduino sono spesso sporchi
                        log.warning("Riga scartata (%d in totale): %s", self.righe_scartate, e)

                if adesso >= prossimo_controllo_config:
                    prossimo_controllo_config = adesso + self.INTERVALLO_CONTROLLO_CONFIG_S
                    self._ricarica_config_se_cambiata()

                if adesso >= prossima_valutazione:
                    prossima_valutazione = adesso + self.INTERVALLO_VALUTAZIONE_S
                    esito = self.valutatore.valuta(adesso, self.cfg)
                    scrivi_stato(self.cfg.file_stato, esito, self.cfg)
                    self.avvisi.aggiorna(esito, adesso, self.cfg.in_taratura,
                                         self.cfg.ripeti_avviso_guasto_min)
                    if self.mostra:
                        print(self._riga_video(esito), flush=True)
                    if adesso >= prossimo_registro:
                        prossimo_registro = adesso + self.cfg.intervallo_registro_s
                        scrivi_registro(self.cfg.cartella_registro, esito, self.cfg)
        finally:
            # Uscendo, lasciamo uno stato NON SICURO: Ekos non deve credere
            # che vada tutto bene mentre il programma e' spento.
            self.sorgente.chiudi()
            scrivi_stato(self.cfg.file_stato, None, self.cfg,
                         motivo_forzato="programma meteo fermato")
            log.info("Stazione meteo fermata.")

    def _riga_video(self, e: Esito) -> str:
        def f(v, u):
            return "  -  " if v is None else f"{v:6.1f}{u}"
        stato = "SICURO" if e.sicuro_calcolato else "NON SICURO"
        if self.cfg.in_taratura:
            stato = f"[taratura] sarebbe {stato}"
        return (f"{datetime.now():%H:%M:%S} aria{f(e.aria, '°')} cielo{f(e.cielo, '°')} "
                f"diff{f(e.differenza, '°')} vento{f(e.vento_medio_kmh, 'km/h')} "
                f"raffica{f(e.raffica_kmh, 'km/h')} pioggia={'?' if e.pioggia is None else ('SI' if e.pioggia else 'no')} "
                f"=> {stato} | {e.testo_motivi()}")


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description="Stazione meteo dell'osservatorio: legge i sensori e informa Ekos.")
    parser.add_argument("--config", default=str(configurazione.percorso_predefinito()),
                        help="percorso del file meteo.ini")
    parser.add_argument("--mostra", action="store_true",
                        help="mostra a video ogni misura (utile per il collaudo)")
    parser.add_argument("--verboso", action="store_true",
                        help="scrive nel log anche ogni riga ricevuta dall'Arduino")
    args = parser.parse_args(argv)

    try:
        cfg = configurazione.carica(args.config)
    except ErroreConfigurazione as e:
        print(f"ERRORE DI CONFIGURAZIONE: {e}", file=sys.stderr)
        return 2

    configura_log(cfg.cartella_registro, args.verboso)
    servizio = Servizio(cfg, mostra=args.mostra)
    signal.signal(signal.SIGTERM, servizio.ferma)
    signal.signal(signal.SIGINT, servizio.ferma)
    servizio.esegui()
    return 0

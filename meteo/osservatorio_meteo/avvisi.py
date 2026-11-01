"""Avvisi per l'utente: notifiche sul desktop e righe nel file di log."""

from __future__ import annotations

import logging
import re
import shutil
import subprocess

log = logging.getLogger("meteo")


class Avvisatore:
    def __init__(self, notifiche_desktop: bool):
        self.notifiche_desktop = notifiche_desktop
        self._notify_send = shutil.which("notify-send")
        if notifiche_desktop and not self._notify_send:
            log.warning("notify-send non trovato: installa il pacchetto 'libnotify-bin' "
                        "per ricevere le notifiche sul desktop")

    def avvisa(self, titolo: str, testo: str, urgente: bool = False) -> None:
        (log.warning if urgente else log.info)("AVVISO: %s - %s", titolo, testo)
        if not (self.notifiche_desktop and self._notify_send):
            return
        comando = [self._notify_send, "--app-name=Osservatorio",
                   "--urgency=critical" if urgente else "--urgency=normal",
                   titolo, testo]
        try:
            subprocess.run(comando, timeout=5, check=False,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except Exception as e:  # una notifica fallita non deve fermare il programma
            log.debug("Notifica non inviata: %s", e)


class GestoreAvvisi:
    """Decide QUANDO avvisare, per non riempire lo schermo di notifiche."""

    def __init__(self, avvisatore: Avvisatore):
        self.a = avvisatore
        self.ultimo_sicuro: bool | None = None
        self.guasti_attivi: tuple[str, ...] = ()
        self.ultimo_avviso_guasto = 0.0
        self.avvisi_attivi = False
        self.ultimo_avviso_controllo = 0.0

    def aggiorna(self, esito, ora: float, in_taratura: bool, ripeti_min: float) -> None:
        ripeti_s = ripeti_min * 60.0

        # Guasti dei sensori: si avvisa SEMPRE, anche in taratura.
        # Per capire se il guasto e' "nuovo" si ignorano i numeri nel testo
        # (contatori e secondi cambiano a ogni misura).
        guasti = tuple(re.sub(r"[\d.,]+", "#", g) for g in esito.guasti)
        if guasti:
            cambiati = guasti != self.guasti_attivi
            if cambiati or ora - self.ultimo_avviso_guasto >= ripeti_s:
                self.a.avvisa("Stazione meteo: GUASTO",
                              "; ".join(esito.guasti) + ". Le condizioni sono considerate NON SICURE.",
                              urgente=True)
                self.ultimo_avviso_guasto = ora
        elif self.guasti_attivi:
            self.a.avvisa("Stazione meteo: guasto risolto", "I sensori rispondono di nuovo.")
        self.guasti_attivi = guasti

        # Cose da controllare (non bloccanti).
        if esito.avvisi:
            if not self.avvisi_attivi or ora - self.ultimo_avviso_controllo >= ripeti_s * 4:
                self.a.avvisa("Stazione meteo: da controllare", "; ".join(esito.avvisi))
                self.ultimo_avviso_controllo = ora
            self.avvisi_attivi = True
        else:
            self.avvisi_attivi = False

        # Cambi sicuro/non sicuro: solo in modalita' normale.
        if in_taratura:
            self.ultimo_sicuro = None
            return
        if self.ultimo_sicuro is None:
            self.ultimo_sicuro = esito.sicuro_comunicato
            return
        if esito.sicuro_comunicato != self.ultimo_sicuro:
            if esito.sicuro_comunicato:
                self.a.avvisa("Meteo: condizioni SICURE", "Le condizioni sono tornate buone.")
            else:
                self.a.avvisa("Meteo: condizioni NON SICURE", esito.testo_motivi(), urgente=True)
            self.ultimo_sicuro = esito.sicuro_comunicato

"""Lettura e controllo del file di configurazione meteo.ini.

Ogni valore viene controllato: se manca o e' scritto male, il programma
spiega in italiano quale riga correggere invece di fermarsi con un errore
incomprensibile.
"""

from __future__ import annotations

import configparser
import os
from dataclasses import dataclass
from pathlib import Path


class ErroreConfigurazione(Exception):
    """Errore nel file meteo.ini, con un messaggio comprensibile."""


VALORI_SI = {"si", "sì", "s", "yes", "y", "true", "1", "on"}
VALORI_NO = {"no", "n", "false", "0", "off"}


@dataclass
class Soglie:
    nuvole_soglia_differenza: float
    nuvole_rendono_non_sicuro: bool
    vento_medio_max_kmh: float
    raffica_max_kmh: float


@dataclass
class Sicurezza:
    attesa_ritorno_sicuro_min: float
    dati_vecchi_dopo_s: float
    stato_vecchio_dopo_s: float
    letture_non_valide_tollerate: int
    aria_min: float
    aria_max: float
    cielo_min: float
    cielo_max: float
    vento_max_plausibile_kmh: float
    avviso_vento_zero_ore: float


@dataclass
class Simulazione:
    aria: float
    cielo: float
    vento_kmh: float
    pioggia: bool
    mlx_funzionante: bool


@dataclass
class Configurazione:
    percorso: Path
    modalita: str                     # "normale" oppure "taratura"
    porta: str
    velocita: int
    soglie: Soglie
    kmh_per_impulso_al_secondo: float
    vento_finestra_media_s: float
    sicurezza: Sicurezza
    notifiche_desktop: bool
    ripeti_avviso_guasto_min: float
    cartella_registro: Path
    intervallo_registro_s: float
    simulazione: Simulazione

    @property
    def in_taratura(self) -> bool:
        return self.modalita == "taratura"

    @property
    def file_stato(self) -> Path:
        return self.cartella_registro / "stato_meteo.json"


class _Lettore:
    """Piccolo aiutante che legge i valori dando messaggi chiari."""

    def __init__(self, parser: configparser.ConfigParser, percorso: Path):
        self.p = parser
        self.percorso = percorso

    def _grezzo(self, sezione: str, chiave: str) -> str:
        if not self.p.has_section(sezione):
            raise ErroreConfigurazione(
                f"Nel file {self.percorso} manca la sezione [{sezione}]."
            )
        if not self.p.has_option(sezione, chiave):
            raise ErroreConfigurazione(
                f"Nel file {self.percorso}, sezione [{sezione}], manca la riga "
                f"'{chiave} = ...'."
            )
        return self.p.get(sezione, chiave).strip()

    def testo(self, sezione: str, chiave: str) -> str:
        return self._grezzo(sezione, chiave)

    def numero(self, sezione: str, chiave: str, minimo=None, massimo=None) -> float:
        testo = self._grezzo(sezione, chiave)
        try:
            valore = float(testo.replace(",", "."))
        except ValueError:
            raise ErroreConfigurazione(
                f"[{sezione}] {chiave} = '{testo}': qui serve un numero (es. 2.4)."
            ) from None
        if minimo is not None and valore < minimo:
            raise ErroreConfigurazione(
                f"[{sezione}] {chiave} = {valore}: il valore minimo ammesso e' {minimo}."
            )
        if massimo is not None and valore > massimo:
            raise ErroreConfigurazione(
                f"[{sezione}] {chiave} = {valore}: il valore massimo ammesso e' {massimo}."
            )
        return valore

    def intero(self, sezione: str, chiave: str, minimo=None, massimo=None) -> int:
        valore = self.numero(sezione, chiave, minimo, massimo)
        if valore != int(valore):
            raise ErroreConfigurazione(
                f"[{sezione}] {chiave} = {valore}: qui serve un numero intero."
            )
        return int(valore)

    def si_no(self, sezione: str, chiave: str) -> bool:
        testo = self._grezzo(sezione, chiave).lower()
        if testo in VALORI_SI:
            return True
        if testo in VALORI_NO:
            return False
        raise ErroreConfigurazione(
            f"[{sezione}] {chiave} = '{testo}': scrivi 'si' oppure 'no'."
        )


def percorso_predefinito() -> Path:
    return Path(os.path.expanduser("~/.config/osservatorio/meteo.ini"))


def carica(percorso: Path | str | None = None) -> Configurazione:
    percorso = Path(percorso) if percorso else percorso_predefinito()
    if not percorso.exists():
        raise ErroreConfigurazione(
            f"Il file di configurazione {percorso} non esiste. "
            f"Copia meteo/meteo.ini.esempio in quella posizione (lo fa strumenti/installa.sh)."
        )
    parser = configparser.ConfigParser(inline_comment_prefixes=(";", "#"))
    try:
        parser.read(percorso, encoding="utf-8")
    except configparser.Error as e:
        raise ErroreConfigurazione(f"Il file {percorso} e' scritto male: {e}") from None

    r = _Lettore(parser, percorso)

    modalita = r.testo("generale", "modalita").lower()
    if modalita not in ("normale", "taratura"):
        raise ErroreConfigurazione(
            f"[generale] modalita = '{modalita}': scrivi 'normale' oppure 'taratura'."
        )

    soglie = Soglie(
        nuvole_soglia_differenza=r.numero("soglie", "nuvole_soglia_differenza", -80, 20),
        nuvole_rendono_non_sicuro=r.si_no("soglie", "nuvole_rendono_non_sicuro"),
        vento_medio_max_kmh=r.numero("soglie", "vento_medio_max_kmh", 0, 200),
        raffica_max_kmh=r.numero("soglie", "raffica_max_kmh", 0, 250),
    )
    sicurezza = Sicurezza(
        attesa_ritorno_sicuro_min=r.numero("sicurezza", "attesa_ritorno_sicuro_min", 0, 240),
        dati_vecchi_dopo_s=r.numero("sicurezza", "dati_vecchi_dopo_s", 5, 3600),
        stato_vecchio_dopo_s=r.numero("sicurezza", "stato_vecchio_dopo_s", 10, 3600),
        letture_non_valide_tollerate=r.intero("sicurezza", "letture_non_valide_tollerate", 0, 100),
        aria_min=r.numero("sicurezza", "aria_min"),
        aria_max=r.numero("sicurezza", "aria_max"),
        cielo_min=r.numero("sicurezza", "cielo_min"),
        cielo_max=r.numero("sicurezza", "cielo_max"),
        vento_max_plausibile_kmh=r.numero("sicurezza", "vento_max_plausibile_kmh", 1),
        avviso_vento_zero_ore=r.numero("sicurezza", "avviso_vento_zero_ore", 0),
    )
    if sicurezza.aria_min >= sicurezza.aria_max:
        raise ErroreConfigurazione("[sicurezza] aria_min deve essere minore di aria_max.")
    if sicurezza.cielo_min >= sicurezza.cielo_max:
        raise ErroreConfigurazione("[sicurezza] cielo_min deve essere minore di cielo_max.")

    simulazione = Simulazione(
        aria=r.numero("simulazione", "aria"),
        cielo=r.numero("simulazione", "cielo"),
        vento_kmh=r.numero("simulazione", "vento_kmh", 0),
        pioggia=r.si_no("simulazione", "pioggia"),
        mlx_funzionante=r.si_no("simulazione", "mlx_funzionante"),
    )

    return Configurazione(
        percorso=percorso,
        modalita=modalita,
        porta=r.testo("seriale", "porta"),
        velocita=r.intero("seriale", "velocita", 300, 1000000),
        soglie=soglie,
        kmh_per_impulso_al_secondo=r.numero("vento", "kmh_per_impulso_al_secondo", 0.01, 100),
        vento_finestra_media_s=r.numero("vento", "vento_finestra_media_s", 10, 3600),
        sicurezza=sicurezza,
        notifiche_desktop=r.si_no("avvisi", "notifiche_desktop"),
        ripeti_avviso_guasto_min=r.numero("avvisi", "ripeti_avviso_guasto_min", 1),
        cartella_registro=Path(os.path.expanduser(r.testo("registro", "cartella"))),
        intervallo_registro_s=r.numero("registro", "intervallo_s", 2, 3600),
        simulazione=simulazione,
    )

"""Da dove arrivano le misure: l'Arduino meteo (porta seriale) o un simulatore.

Il simulatore serve per provare tutto il sistema (fino a Ekos) senza avere
ancora i sensori: i valori si scrivono nella sezione [simulazione] di meteo.ini.
"""

from __future__ import annotations

import json
import logging
import time
from typing import Callable

from .configurazione import Configurazione

log = logging.getLogger("meteo")


class SorgenteSeriale:
    """Legge le righe inviate dall'Arduino meteo tramite USB."""

    ATTESA_RICONNESSIONE_S = 5.0

    def __init__(self, porta: str, velocita: int):
        self.porta = porta
        self.velocita = velocita
        self._seriale = None
        self._ultimo_tentativo = 0.0
        self.errore: str | None = "porta non ancora aperta"

    def _apri(self) -> None:
        import serial  # pacchetto python3-serial

        ora = time.monotonic()
        if ora - self._ultimo_tentativo < self.ATTESA_RICONNESSIONE_S:
            time.sleep(0.5)
            return
        self._ultimo_tentativo = ora
        try:
            self._seriale = serial.Serial(self.porta, self.velocita, timeout=1.0)
            self.errore = None
            log.info("Porta %s aperta", self.porta)
        except (OSError, serial.SerialException) as e:
            self._seriale = None
            nuovo = f"impossibile aprire {self.porta}: {e}"
            if nuovo != self.errore:
                log.error(nuovo)
            self.errore = nuovo

    def leggi_riga(self) -> str | None:
        """Restituisce una riga (senza a capo) oppure None se non e' arrivato nulla."""
        if self._seriale is None:
            self._apri()
            if self._seriale is None:
                return None
        try:
            dati = self._seriale.readline()
        except Exception as e:  # cavo staccato durante la lettura, ecc.
            log.error("Errore di lettura da %s: %s", self.porta, e)
            self.errore = f"errore di lettura: {e}"
            self.chiudi()
            return None
        if not dati:
            return None
        return dati.decode("ascii", errors="replace").strip()

    def chiudi(self) -> None:
        if self._seriale is not None:
            try:
                self._seriale.close()
            except Exception:
                pass
        self._seriale = None


class SorgenteSimulata:
    """Genera una misura ogni 2 secondi con i valori di [simulazione]."""

    def __init__(self, configurazione: Callable[[], Configurazione]):
        self._cfg = configurazione
        self._numero = 0
        self._ultima = time.monotonic()
        self.errore: str | None = None

    def leggi_riga(self) -> str | None:
        time.sleep(0.2)
        ora = time.monotonic()
        if ora - self._ultima < 2.0:
            return None
        durata_ms = int((ora - self._ultima) * 1000)
        self._ultima = ora
        self._numero += 1
        s = self._cfg().simulazione
        kmh_per_hz = self._cfg().kmh_per_impulso_al_secondo
        impulsi = round(s.vento_kmh / kmh_per_hz * durata_ms / 1000.0)
        dati = {
            "id": "oss-meteo", "v": 1, "n": self._numero, "ms": durata_ms,
            "ta": s.aria if s.mlx_funzionante else None,
            "tc": s.cielo if s.mlx_funzionante else None,
            "mlx": 1 if s.mlx_funzionante else 0,
            "imp": impulsi,
            "piog": 1 if s.pioggia else 0,
        }
        return json.dumps(dati)

    def chiudi(self) -> None:
        pass

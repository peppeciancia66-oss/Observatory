"""Il "cervello" della stazione meteo: decide se le condizioni sono sicure.

Questo modulo non legge porte seriali e non scrive file: riceve le misure
e restituisce un esito. Cosi' si puo' provare a fondo con i test automatici
(cartella tests/).

Regole principali:
  * pioggia                                  -> NON SICURO
  * vento medio o raffica oltre soglia       -> NON SICURO
  * nuvole oltre soglia (se abilitato)       -> NON SICURO
  * nessun dato, dati vecchi, valori assurdi,
    sensore che non risponde                 -> NON SICURO + GUASTO (avviso)
  * dopo un periodo non sicuro, si torna "sicuro" solo quando le condizioni
    sono rimaste buone per "attesa_ritorno_sicuro_min" minuti di fila.
"""

from __future__ import annotations

import json
import math
from collections import deque
from dataclasses import dataclass, field

from .configurazione import Configurazione

ID_DISPOSITIVO = "oss-meteo"
INTERVALLO_MIN_MS = 500
INTERVALLO_MAX_MS = 10000


class RigaNonValida(ValueError):
    """La riga ricevuta dall'Arduino non e' nel formato atteso."""


@dataclass
class Misura:
    """Una misura inviata dall'Arduino meteo (una riga JSON)."""
    numero: int
    durata_ms: int
    aria: float | None
    cielo: float | None
    mlx_ok: bool
    impulsi: int
    pioggia: bool


def _numero_o_none(valore, nome):
    if valore is None:
        return None
    if isinstance(valore, bool) or not isinstance(valore, (int, float)):
        raise RigaNonValida(f"campo '{nome}' non numerico")
    if math.isnan(valore) or math.isinf(valore):
        return None
    return float(valore)


def interpreta_riga(testo: str) -> Misura:
    """Trasforma una riga di testo dell'Arduino in una Misura.

    Solleva RigaNonValida se la riga e' rovinata o non viene dall'Arduino meteo.
    """
    testo = testo.strip()
    if not testo:
        raise RigaNonValida("riga vuota")
    try:
        dati = json.loads(testo)
    except json.JSONDecodeError:
        raise RigaNonValida(f"riga illeggibile: {testo[:60]!r}") from None
    if not isinstance(dati, dict):
        raise RigaNonValida("la riga non e' un oggetto JSON")
    if dati.get("id") != ID_DISPOSITIVO:
        raise RigaNonValida(f"dispositivo sconosciuto: {dati.get('id')!r}")
    if dati.get("v") != 1:
        raise RigaNonValida(f"versione del formato non supportata: {dati.get('v')!r}")
    try:
        numero = int(dati["n"])
        durata = int(dati["ms"])
        impulsi = int(dati["imp"])
        mlx_ok = int(dati["mlx"]) == 1
        pioggia = int(dati["piog"]) == 1
    except (KeyError, TypeError, ValueError) as e:
        raise RigaNonValida(f"campo mancante o errato: {e}") from None
    if impulsi < 0:
        raise RigaNonValida("numero di impulsi negativo")
    aria = _numero_o_none(dati.get("ta"), "ta")
    cielo = _numero_o_none(dati.get("tc"), "tc")
    if aria is None or cielo is None:
        mlx_ok = False
    return Misura(numero, durata, aria, cielo, mlx_ok, impulsi, pioggia)


@dataclass
class Esito:
    """Risultato di una valutazione."""
    condizioni_buone: bool            # in questo istante, senza attesa
    sicuro_calcolato: bool            # dopo l'attesa di ritorno al sicuro
    sicuro_comunicato: bool           # cio' che si dice a Ekos (False in taratura)
    motivi: list[str] = field(default_factory=list)   # perche' non e' sicuro
    guasti: list[str] = field(default_factory=list)   # problemi dei sensori
    avvisi: list[str] = field(default_factory=list)   # cose da controllare
    aria: float | None = None
    cielo: float | None = None
    differenza: float | None = None
    vento_medio_kmh: float | None = None
    raffica_kmh: float | None = None
    pioggia: bool | None = None

    def testo_motivi(self) -> str:
        tutti = [f"GUASTO: {g}" for g in self.guasti] + self.motivi
        return "; ".join(tutti) if tutti else "condizioni buone"


class Valutatore:
    """Accumula le misure e calcola l'esito."""

    def __init__(self, avvio: float):
        self.avvio = avvio
        self.ultima_ricezione: float | None = None
        self.letture_mlx_non_valide = 0
        self.motivo_mlx_non_valido = ""
        self.ultima_mlx: tuple[float, float] | None = None  # (aria, cielo)
        self.pioggia: bool | None = None
        self.vento: deque[tuple[float, float, float]] = deque()  # (ora, secondi, kmh)
        self.ultimo_vento_assurdo: float | None = None
        self.valore_vento_assurdo = 0.0
        self.ultimo_impulso = avvio
        self.buone_da: float | None = None
        self.sicuro = False

    # ------------------------------------------------------------------
    def aggiungi(self, m: Misura, ora: float, cfg: Configurazione) -> None:
        sic = cfg.sicurezza
        self.ultima_ricezione = ora
        self.pioggia = m.pioggia

        # --- Sensore a infrarossi ---
        if not m.mlx_ok:
            self.letture_mlx_non_valide += 1
            self.motivo_mlx_non_valido = "il sensore a infrarossi non risponde o da' dati rovinati"
        elif not (sic.aria_min <= m.aria <= sic.aria_max):
            self.letture_mlx_non_valide += 1
            self.motivo_mlx_non_valido = (
                f"temperatura aria assurda ({m.aria:.1f} °C, ammessa da "
                f"{sic.aria_min:g} a {sic.aria_max:g})"
            )
        elif not (sic.cielo_min <= m.cielo <= sic.cielo_max):
            self.letture_mlx_non_valide += 1
            self.motivo_mlx_non_valido = (
                f"temperatura cielo assurda ({m.cielo:.1f} °C, ammessa da "
                f"{sic.cielo_min:g} a {sic.cielo_max:g})"
            )
        else:
            self.letture_mlx_non_valide = 0
            self.ultima_mlx = (m.aria, m.cielo)

        # --- Anemometro ---
        if INTERVALLO_MIN_MS <= m.durata_ms <= INTERVALLO_MAX_MS:
            secondi = m.durata_ms / 1000.0
            kmh = m.impulsi / secondi * cfg.kmh_per_impulso_al_secondo
            if kmh > sic.vento_max_plausibile_kmh:
                self.ultimo_vento_assurdo = ora
                self.valore_vento_assurdo = kmh
            else:
                self.vento.append((ora, secondi, kmh))
            if m.impulsi > 0:
                self.ultimo_impulso = ora
        self._pulisci_vento(ora, cfg)

    def _pulisci_vento(self, ora: float, cfg: Configurazione) -> None:
        limite = ora - cfg.vento_finestra_media_s
        while self.vento and self.vento[0][0] < limite:
            self.vento.popleft()

    # ------------------------------------------------------------------
    def valuta(self, ora: float, cfg: Configurazione) -> Esito:
        sic = cfg.sicurezza
        soglie = cfg.soglie
        e = Esito(condizioni_buone=False, sicuro_calcolato=False, sicuro_comunicato=False)
        self._pulisci_vento(ora, cfg)

        # 1) Arrivano dati?
        if self.ultima_ricezione is None:
            if ora - self.avvio > sic.dati_vecchi_dopo_s:
                e.guasti.append(
                    f"nessun dato dalla stazione meteo da {ora - self.avvio:.0f} s "
                    f"(Arduino scollegato o porta sbagliata?)"
                )
            else:
                e.motivi.append("in attesa delle prime misure")
        elif ora - self.ultima_ricezione > sic.dati_vecchi_dopo_s:
            e.guasti.append(
                f"la stazione meteo non invia dati da {ora - self.ultima_ricezione:.0f} s"
            )
        else:
            # 2) Pioggia
            e.pioggia = self.pioggia
            if self.pioggia:
                e.motivi.append("pioggia")

            # 3) Nuvole (sensore a infrarossi)
            if self.letture_mlx_non_valide > sic.letture_non_valide_tollerate:
                e.guasti.append(
                    f"{self.motivo_mlx_non_valido} "
                    f"({self.letture_mlx_non_valide} letture di fila)"
                )
            elif self.ultima_mlx is None:
                e.motivi.append("in attesa di una lettura valida del sensore a infrarossi")
            else:
                e.aria, e.cielo = self.ultima_mlx
                e.differenza = e.cielo - e.aria
                if soglie.nuvole_rendono_non_sicuro and e.differenza > soglie.nuvole_soglia_differenza:
                    e.motivi.append(
                        f"nuvoloso (differenza cielo-aria {e.differenza:.1f} °C, "
                        f"soglia {soglie.nuvole_soglia_differenza:g})"
                    )

            # 4) Vento
            if (self.ultimo_vento_assurdo is not None
                    and ora - self.ultimo_vento_assurdo <= cfg.vento_finestra_media_s):
                e.guasti.append(
                    f"anemometro: valore assurdo ({self.valore_vento_assurdo:.0f} km/h)"
                )
            elif not self.vento:
                e.motivi.append("in attesa di misure del vento")
            else:
                secondi_tot = sum(s for _, s, _ in self.vento)
                e.vento_medio_kmh = sum(k * s for _, s, k in self.vento) / secondi_tot
                e.raffica_kmh = max(k for _, _, k in self.vento)
                if e.vento_medio_kmh > soglie.vento_medio_max_kmh:
                    e.motivi.append(
                        f"vento medio {e.vento_medio_kmh:.0f} km/h "
                        f"(soglia {soglie.vento_medio_max_kmh:g})"
                    )
                if e.raffica_kmh > soglie.raffica_max_kmh:
                    e.motivi.append(
                        f"raffica {e.raffica_kmh:.0f} km/h (soglia {soglie.raffica_max_kmh:g})"
                    )

            if sic.avviso_vento_zero_ore > 0:
                ore_ferme = (ora - self.ultimo_impulso) / 3600.0
                if ore_ferme > sic.avviso_vento_zero_ore:
                    e.avvisi.append(
                        f"l'anemometro segna zero da {ore_ferme:.0f} ore: "
                        f"controlla che giri e che sia collegato"
                    )

        # 5) Attesa prima di tornare "sicuro"
        e.condizioni_buone = not e.motivi and not e.guasti
        if not e.condizioni_buone:
            self.buone_da = None
            self.sicuro = False
        else:
            if self.buone_da is None:
                self.buone_da = ora
            attesa_s = sic.attesa_ritorno_sicuro_min * 60.0
            if not self.sicuro and ora - self.buone_da >= attesa_s:
                self.sicuro = True
            if not self.sicuro:
                mancano = (attesa_s - (ora - self.buone_da)) / 60.0
                e.motivi.append(
                    f"condizioni buone, attendo ancora {mancano:.1f} min prima di dichiarare sicuro"
                )

        e.sicuro_calcolato = self.sicuro
        e.sicuro_comunicato = self.sicuro and not cfg.in_taratura
        return e

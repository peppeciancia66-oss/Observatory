import json

import pytest

from conftest import imposta
from osservatorio_meteo import configurazione
from osservatorio_meteo.valutazione import Misura, RigaNonValida, Valutatore, interpreta_riga


def riga(**cambi):
    d = {"id": "oss-meteo", "v": 1, "n": 1, "ms": 2000, "ta": 10.0, "tc": -25.0,
         "mlx": 1, "imp": 2, "piog": 0}
    d.update(cambi)
    return json.dumps(d)


def misura(**cambi):
    return interpreta_riga(riga(**cambi))


def alimenta(v, cfg, t0, durata_s, **cambi):
    """Come il servizio vero: ogni 2 s una misura e una valutazione.
    Restituisce l'ora finale."""
    t = t0
    while t < t0 + durata_s:
        t += 2
        v.aggiungi(misura(**cambi), t, cfg)
        v.valuta(t, cfg)
    return t


# ---------------- interpretazione delle righe ----------------

def test_riga_valida():
    m = interpreta_riga(riga())
    assert m == Misura(1, 2000, 10.0, -25.0, True, 2, False)


def test_riga_mlx_null():
    m = interpreta_riga(riga(ta=None, tc=None, mlx=0))
    assert not m.mlx_ok and m.aria is None


@pytest.mark.parametrize("testo", [
    "", "spazzatura", "{\"id\":\"altro\",\"v\":1}", riga(v=2), riga(imp=-1),
    riga(ta="caldo"), "[1,2,3]", '{"id":"oss-meteo","v":1}',
])
def test_righe_non_valide(testo):
    with pytest.raises(RigaNonValida):
        interpreta_riga(testo)


# ---------------- valutazione ----------------

def test_sereno_diventa_sicuro_solo_dopo_attesa(cfg):
    v = Valutatore(0)
    t = alimenta(v, cfg, 0, 60)
    e = v.valuta(t, cfg)
    assert e.condizioni_buone and not e.sicuro_calcolato
    assert "attendo" in e.testo_motivi()
    t = alimenta(v, cfg, t, cfg.sicurezza.attesa_ritorno_sicuro_min * 60)
    e = v.valuta(t, cfg)
    assert e.sicuro_calcolato and e.sicuro_comunicato
    assert e.differenza == pytest.approx(-35.0)


def test_attesa_zero(cfg):
    cfg.sicurezza.attesa_ritorno_sicuro_min = 0
    v = Valutatore(0)
    t = alimenta(v, cfg, 0, 10)
    assert v.valuta(t, cfg).sicuro_comunicato


@pytest.fixture
def sicuro(cfg):
    """Valutatore gia' in stato sicuro."""
    cfg.sicurezza.attesa_ritorno_sicuro_min = 1
    v = Valutatore(0)
    t = alimenta(v, cfg, 0, 70)
    assert v.valuta(t, cfg).sicuro_comunicato
    return v, t


def test_pioggia_immediatamente_non_sicuro(cfg, sicuro):
    v, t = sicuro
    v.aggiungi(misura(piog=1), t + 2, cfg)
    e = v.valuta(t + 2, cfg)
    assert not e.sicuro_comunicato and "pioggia" in e.motivi


def test_nuvole(cfg, sicuro):
    v, t = sicuro
    v.aggiungi(misura(tc=0.0), t + 2, cfg)          # differenza -10 > soglia -15
    e = v.valuta(t + 2, cfg)
    assert not e.sicuro_comunicato and "nuvoloso" in e.testo_motivi()


def test_nuvole_disattivabili(cfg, sicuro):
    v, t = sicuro
    cfg.soglie.nuvole_rendono_non_sicuro = False
    v.aggiungi(misura(tc=0.0), t + 2, cfg)
    assert v.valuta(t + 2, cfg).sicuro_comunicato


def test_vento_medio(cfg, sicuro):
    v, t = sicuro
    # 2.4 km/h per impulso/s: 24 impulsi in 2 s = 28.8 km/h > 25
    t = alimenta(v, cfg, t, cfg.vento_finestra_media_s + 2, imp=24)
    e = v.valuta(t, cfg)
    assert e.vento_medio_kmh == pytest.approx(28.8)
    assert not e.sicuro_comunicato and "vento medio" in e.testo_motivi()


def test_raffica(cfg, sicuro):
    v, t = sicuro
    v.aggiungi(misura(imp=40), t + 2, cfg)            # 48 km/h > 40
    e = v.valuta(t + 2, cfg)
    assert not e.sicuro_comunicato and "raffica" in e.testo_motivi()


def test_vento_assurdo_e_guasto(cfg, sicuro):
    v, t = sicuro
    v.aggiungi(misura(imp=500), t + 2, cfg)           # 600 km/h
    e = v.valuta(t + 2, cfg)
    assert not e.sicuro_comunicato and e.guasti and "anemometro" in e.guasti[0]


def test_mlx_poche_letture_errate_tollerate(cfg, sicuro):
    v, t = sicuro
    for i in range(cfg.sicurezza.letture_non_valide_tollerate):
        t += 2
        v.aggiungi(misura(ta=None, tc=None, mlx=0), t, cfg)
    assert v.valuta(t, cfg).sicuro_comunicato


def test_mlx_guasto(cfg, sicuro):
    v, t = sicuro
    for i in range(cfg.sicurezza.letture_non_valide_tollerate + 1):
        t += 2
        v.aggiungi(misura(ta=None, tc=None, mlx=0), t, cfg)
    e = v.valuta(t, cfg)
    assert not e.sicuro_comunicato and "infrarossi" in e.guasti[0]


def test_temperatura_assurda(cfg, sicuro):
    v, t = sicuro
    for i in range(cfg.sicurezza.letture_non_valide_tollerate + 1):
        t += 2
        v.aggiungi(misura(tc=1037.0), t, cfg)
    e = v.valuta(t, cfg)
    assert not e.sicuro_comunicato and "assurda" in e.guasti[0]


def test_dati_vecchi(cfg, sicuro):
    v, t = sicuro
    e = v.valuta(t + cfg.sicurezza.dati_vecchi_dopo_s + 1, cfg)
    assert not e.sicuro_comunicato and "non invia dati" in e.guasti[0]


def test_nessun_dato_mai(cfg):
    v = Valutatore(0)
    assert v.valuta(5, cfg).motivi == ["in attesa delle prime misure"]
    e = v.valuta(cfg.sicurezza.dati_vecchi_dopo_s + 1, cfg)
    assert e.guasti and not e.sicuro_comunicato


def test_ritorno_sicuro_richiede_nuova_attesa(cfg, sicuro):
    v, t = sicuro
    v.aggiungi(misura(piog=1), t + 2, cfg)
    v.valuta(t + 2, cfg)
    t = alimenta(v, cfg, t + 2, 30)                    # asciutto da 30 s soltanto
    assert not v.valuta(t, cfg).sicuro_comunicato
    t = alimenta(v, cfg, t, 40)                        # ora > 1 minuto
    assert v.valuta(t, cfg).sicuro_comunicato


def test_taratura_non_comunica_mai_sicuro(ini):
    cfg = configurazione.carica(ini)                   # l'esempio e' in taratura
    cfg.sicurezza.attesa_ritorno_sicuro_min = 0
    v = Valutatore(0)
    t = alimenta(v, cfg, 0, 10)
    e = v.valuta(t, cfg)
    assert e.sicuro_calcolato and not e.sicuro_comunicato


def test_avviso_anemometro_fermo(cfg):
    cfg.sicurezza.avviso_vento_zero_ore = 1
    v = Valutatore(0)
    t = alimenta(v, cfg, 0, 3700, imp=0)
    assert v.valuta(t, cfg).avvisi


def test_durata_intervallo_assurda_ignorata_per_il_vento(cfg):
    v = Valutatore(0)
    v.aggiungi(misura(ms=50, imp=100), 2, cfg)
    assert not v.vento

import pytest

from conftest import imposta
from osservatorio_meteo import configurazione
from osservatorio_meteo.configurazione import ErroreConfigurazione


def test_esempio_valido_e_in_taratura(ini):
    cfg = configurazione.carica(ini)
    assert cfg.modalita == "taratura"
    assert cfg.in_taratura
    assert cfg.soglie.vento_medio_max_kmh == 25
    assert cfg.kmh_per_impulso_al_secondo == 2.4


def test_virgola_decimale_accettata(ini):
    imposta(ini, "kmh_per_impulso_al_secondo", "2,4")
    assert configurazione.carica(ini).kmh_per_impulso_al_secondo == 2.4


@pytest.mark.parametrize("chiave,valore,frammento", [
    ("modalita", "automatica", "normale"),
    ("vento_medio_max_kmh", "tanto", "numero"),
    ("nuvole_rendono_non_sicuro", "forse", "si"),
    ("vento_medio_max_kmh", "-5", "minimo"),
])
def test_errori_spiegati(ini, chiave, valore, frammento):
    imposta(ini, chiave, valore)
    with pytest.raises(ErroreConfigurazione) as e:
        configurazione.carica(ini)
    assert frammento in str(e.value)


def test_file_mancante(tmp_path):
    with pytest.raises(ErroreConfigurazione):
        configurazione.carica(tmp_path / "non_esiste.ini")

import json
import os
import subprocess
import sys
import time

import pytest

from conftest import CARTELLA_METEO, imposta
from osservatorio_meteo import configurazione
from osservatorio_meteo.servizio import scrivi_stato
from osservatorio_meteo.valutazione import Esito

SCRIPT = CARTELLA_METEO / "weather_status.py"


def esegui(ini):
    """Esegue lo script come farebbe il driver INDI e decodifica il JSON."""
    amb = dict(os.environ, OSSERVATORIO_METEO_INI=str(ini))
    r = subprocess.run([str(SCRIPT)], capture_output=True, text=True, env=amb, timeout=10)
    assert r.returncode == 0
    return json.loads(r.stdout)["roof_status"]


def esito(sicuro=True):
    return Esito(condizioni_buone=sicuro, sicuro_calcolato=sicuro, sicuro_comunicato=sicuro)


def test_file_assente(ini):
    assert esegui(ini)["open_ok"] == 0


def test_sicuro(cfg, ini):
    scrivi_stato(cfg.file_stato, esito(True), cfg)
    r = esegui(ini)
    assert r["open_ok"] == 1


def test_non_sicuro(cfg, ini):
    e = esito(False)
    e.motivi.append("pioggia")
    scrivi_stato(cfg.file_stato, e, cfg)
    r = esegui(ini)
    assert r["open_ok"] == 0 and "pioggia" in r["reasons"]


def test_taratura_sempre_non_sicuro(ini):
    cfg = configurazione.carica(ini)
    scrivi_stato(cfg.file_stato, esito(True), cfg)
    r = esegui(ini)
    assert r["open_ok"] == 0 and "TARATURA" in r["reasons"]


def test_stato_vecchio(cfg, ini):
    scrivi_stato(cfg.file_stato, esito(True), cfg)
    dati = json.loads(cfg.file_stato.read_text())
    dati["aggiornato"] = time.time() - 1000
    cfg.file_stato.write_text(json.dumps(dati))
    r = esegui(ini)
    assert r["open_ok"] == 0 and "fermo" in r["reasons"]


@pytest.mark.parametrize("contenuto", ["", "{rotto", "[]", '{"sicuro": true}',
                                       '{"aggiornato": "ieri", "sicuro": true, "modalita": "normale"}'])
def test_file_rovinato(cfg, ini, contenuto):
    cfg.file_stato.parent.mkdir(parents=True, exist_ok=True)
    cfg.file_stato.write_text(contenuto)
    assert esegui(ini)["open_ok"] == 0


def test_programma_fermato(cfg, ini):
    scrivi_stato(cfg.file_stato, None, cfg, motivo_forzato="programma meteo fermato")
    r = esegui(ini)
    assert r["open_ok"] == 0 and "fermato" in r["reasons"]


def test_config_rovinata_usa_valori_prudenti(cfg, ini):
    scrivi_stato(cfg.file_stato, esito(True), cfg)
    imposta(ini, "stato_vecchio_dopo_s", "boh")
    assert esegui(ini)["open_ok"] == 1   # stato fresco: 90 s predefiniti bastano

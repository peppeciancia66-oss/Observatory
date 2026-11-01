import shutil
import sys
from pathlib import Path

import pytest

CARTELLA_METEO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(CARTELLA_METEO))

from osservatorio_meteo import configurazione  # noqa: E402


@pytest.fixture
def ini(tmp_path):
    """Copia di meteo.ini.esempio in una cartella temporanea, modificabile nei test."""
    percorso = tmp_path / "meteo.ini"
    shutil.copy(CARTELLA_METEO / "meteo.ini.esempio", percorso)
    testo = percorso.read_text(encoding="utf-8")
    testo = testo.replace("cartella = ~/osservatorio-meteo", f"cartella = {tmp_path / 'dati'}")
    percorso.write_text(testo, encoding="utf-8")
    return percorso


def imposta(percorso, chiave, valore):
    """Cambia 'chiave = ...' nel file ini (la prima occorrenza)."""
    righe = percorso.read_text(encoding="utf-8").splitlines()
    for i, r in enumerate(righe):
        if r.split("=")[0].strip() == chiave:
            righe[i] = f"{chiave} = {valore}"
            break
    else:
        raise KeyError(chiave)
    percorso.write_text("\n".join(righe) + "\n", encoding="utf-8")


@pytest.fixture
def cfg(ini):
    imposta(ini, "modalita", "normale")
    return configurazione.carica(ini)

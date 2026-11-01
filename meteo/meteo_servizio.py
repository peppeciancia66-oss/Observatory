#!/usr/bin/env python3
"""Avvia la stazione meteo. Uso:

    python3 meteo_servizio.py                 (usa ~/.config/osservatorio/meteo.ini)
    python3 meteo_servizio.py --mostra        (mostra le misure a video)
    python3 meteo_servizio.py --config ALTRO.ini
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from osservatorio_meteo.servizio import main  # noqa: E402

if __name__ == "__main__":
    sys.exit(main())

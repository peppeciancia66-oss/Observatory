#!/usr/bin/env python3
"""Riassunto dei dati registrati durante la taratura.

Per ogni NOTTE registrata (da mezzogiorno a mezzogiorno, contando solo le
ore fra le 19 e le 6, perche' di giorno il sole falsa il sensore) mostra:
  - la differenza cielo-aria (minima, mediana, massima): e' il numero che
    serve per scegliere la soglia delle nuvole;
  - il vento medio e la raffica piu' alti;
  - per quanta parte del tempo ha piovuto;
  - con le soglie ATTUALI, per quanta parte del tempo sarebbe stato "nuvoloso".

Uso:
    python3 riepilogo_taratura.py                 (cartella ~/osservatorio-meteo/registro)
    python3 riepilogo_taratura.py ALTRA_CARTELLA

Consiglio: accanto a ogni data annota su un quaderno com'era il cielo quella
notte (sereno / velato / coperto). Confrontando le note con i numeri trovi
la soglia giusta per il tuo cielo (vedi docs/08-taratura-meteo.md).
"""

import configparser
import csv
import os
import statistics
import sys
from collections import defaultdict
from datetime import datetime, timedelta
from pathlib import Path

ORA_INIZIO_NOTTE = 19
ORA_FINE_NOTTE = 6


def numero(testo):
    testo = (testo or "").strip().replace(",", ".")
    try:
        return float(testo)
    except ValueError:
        return None


def soglia_nuvole_attuale():
    p = configparser.ConfigParser(inline_comment_prefixes=(";", "#"))
    p.read(os.path.expanduser("~/.config/osservatorio/meteo.ini"), encoding="utf-8")
    return numero(p.get("soglie", "nuvole_soglia_differenza", fallback=""))


def main():
    cartella = Path(sys.argv[1] if len(sys.argv) > 1
                    else os.path.expanduser("~/osservatorio-meteo/registro"))
    file_csv = sorted(cartella.glob("*.csv"))
    if not file_csv:
        print(f"Nessun file CSV in {cartella}")
        return 1
    soglia = soglia_nuvole_attuale()
    print(f"Soglia nuvole attuale: {soglia if soglia is not None else '(non letta)'}\n")
    notti = defaultdict(list)
    for f in file_csv:
        with open(f, encoding="utf-8") as fp:
            for r in csv.DictReader(fp, delimiter=";"):
                try:
                    quando = datetime.fromisoformat(r["data_ora"])
                except (KeyError, ValueError):
                    continue
                if ORA_FINE_NOTTE <= quando.hour < ORA_INIZIO_NOTTE:
                    continue
                notte = (quando - timedelta(hours=12)).date()
                notti[notte].append(r)
    if not notti:
        print("Nessuna misura notturna registrata (fra le 19 e le 6).")
        return 1

    def fmt(v):
        return f"{v:.1f}" if v is not None else "-"

    print(f"{'notte del':<12}{'righe':>6}{'diff min':>10}{'mediana':>9}{'max':>7}"
          f"{'vento max':>11}{'raffica':>9}{'pioggia':>9}{'nuvoloso':>10}")
    for notte in sorted(notti):
        righe = notti[notte]
        diff = [d for d in (numero(r.get("differenza")) for r in righe) if d is not None]
        vento = [v for v in (numero(r.get("vento_medio_kmh")) for r in righe) if v is not None]
        raffica = [v for v in (numero(r.get("raffica_kmh")) for r in righe) if v is not None]
        pioggia = [r for r in righe if r.get("pioggia") == "si"]
        nuvoloso = "-"
        if diff and soglia is not None:
            nuvoloso = f"{100 * sum(d > soglia for d in diff) / len(diff):.0f}%"
        print(f"{notte.isoformat():<12}{len(righe):>6}"
              f"{fmt(min(diff) if diff else None):>10}"
              f"{fmt(statistics.median(diff) if diff else None):>9}"
              f"{fmt(max(diff) if diff else None):>7}"
              f"{fmt(max(vento) if vento else None):>11}"
              f"{fmt(max(raffica) if raffica else None):>9}"
              f"{f'{100 * len(pioggia) / len(righe):.0f}%':>9}"
              f"{nuvoloso:>10}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

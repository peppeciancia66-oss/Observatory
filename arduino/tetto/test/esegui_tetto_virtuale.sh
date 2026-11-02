#!/bin/bash
# Avvia l'"Arduino virtuale" del tetto (vedi tetto_virtuale.cpp). Serve g++.
set -e
cd "$(dirname "$0")"
CARTELLA=$(mktemp -d)
trap 'rm -rf "$CARTELLA"' EXIT
g++ -std=c++17 -O1 -I finto_arduino -DTETTO_INO="\"$(cd .. && pwd)/tetto.ino\"" \
    tetto_virtuale.cpp -o "$CARTELLA/tetto_virtuale"
"$CARTELLA/tetto_virtuale" "${1:-/tmp/tetto-virtuale}"

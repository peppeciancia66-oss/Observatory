#!/bin/bash
# Compila tetto.ino per il PC e lo mette alla prova con il simulatore.
# Serve solo il compilatore C++ (pacchetto g++). Non serve nessun Arduino.
set -e
cd "$(dirname "$0")"
CARTELLA=$(mktemp -d)
trap 'rm -rf "$CARTELLA"' EXIT

echo "== Versione normale (chiusura autonoma per pioggia DISATTIVATA) =="
g++ -std=c++17 -Wall -Wno-unused-function -I finto_arduino \
    -DTETTO_INO="\"$(pwd)/../tetto.ino\"" prova_tetto.cpp -o "$CARTELLA/prova"
"$CARTELLA/prova"

echo
echo "== Versione di prova con chiusura autonoma per pioggia ATTIVA =="
sed 's/^#define CHIUSURA_AUTONOMA_PIOGGIA   0/#define CHIUSURA_AUTONOMA_PIOGGIA   1/' ../tetto.ino > "$CARTELLA/tetto_autonomo.ino"
grep -q '^#define CHIUSURA_AUTONOMA_PIOGGIA   1' "$CARTELLA/tetto_autonomo.ino"
g++ -std=c++17 -Wall -Wno-unused-function -I finto_arduino \
    -DTETTO_INO="\"$CARTELLA/tetto_autonomo.ino\"" prova_tetto.cpp -o "$CARTELLA/prova_autonomo"
"$CARTELLA/prova_autonomo"

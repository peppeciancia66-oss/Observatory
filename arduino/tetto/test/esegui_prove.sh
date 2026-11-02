#!/bin/bash
# Compila tetto.ino per il PC e lo mette alla prova con il simulatore.
# Serve solo il compilatore C++ (pacchetto g++). Non serve nessun Arduino.
# Si provano tre versioni: quella normale e due varianti con funzioni
# facoltative attivate (chiusura autonoma per pioggia, sensore di movimento).
set -e
cd "$(dirname "$0")"
CARTELLA=$(mktemp -d)
trap 'rm -rf "$CARTELLA"' EXIT

prova() {   # $1 = titolo, $2 = sottocartella con tetto.ino e configurazione_scheda.h
    echo
    echo "== $1 =="
    ESEGUIBILE=$(mktemp -p "$CARTELLA")
    g++ -std=c++17 -Wall -Wno-unused-function -I finto_arduino \
        -DTETTO_INO="\"$2/tetto.ino\"" prova_tetto.cpp -o "$ESEGUIBILE"
    "$ESEGUIBILE"
}

variante() {  # $1 = nome, $2 = file da modificare, $3 = riga originale, $4 = riga nuova
    mkdir -p "$CARTELLA/$1"
    cp ../tetto.ino ../configurazione_scheda.h "$CARTELLA/$1/"
    sed -i "s/^$3\$/$4/" "$CARTELLA/$1/$2"
    grep -q "^$4\$" "$CARTELLA/$1/$2"
}

prova "Versione normale (come verra' caricata sull'Arduino)" "$(cd .. && pwd)"

variante pioggia tetto.ino '#define CHIUSURA_AUTONOMA_PIOGGIA   0' '#define CHIUSURA_AUTONOMA_PIOGGIA   1'
prova "Variante: chiusura autonoma per pioggia ATTIVA" "$CARTELLA/pioggia"

variante movimento configurazione_scheda.h '#define USA_SENSORE_MOVIMENTO  0' '#define USA_SENSORE_MOVIMENTO  1'
prova "Variante: sensore di movimento ATTIVO" "$CARTELLA/movimento"

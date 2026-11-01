#!/bin/bash
# =============================================================================
#  crea_regola_udev.sh  -  Dà un NOME FISSO a un Arduino collegato via USB
# =============================================================================
#
#  Perché serve: Linux chiama le porte USB /dev/ttyACM0, /dev/ttyUSB0, ...
#  in base all'ordine in cui i dispositivi si accendono. Con montatura,
#  focheggiatore e due Arduino, i numeri possono cambiare a ogni avvio e Ekos
#  rischierebbe di parlare col dispositivo sbagliato.
#  Questo script crea una "regola udev" che dà sempre lo stesso nome:
#      /dev/osservatorio-tetto   oppure   /dev/osservatorio-meteo
#
#  Uso (un Arduino alla volta):
#      bash crea_regola_udev.sh tetto
#      bash crea_regola_udev.sh meteo
#  Lo script ti chiederà di scollegare e ricollegare l'Arduino.
# =============================================================================

set -e

NOME="$1"
if [ "$NOME" != "tetto" ] && [ "$NOME" != "meteo" ]; then
    echo "Uso: bash $0 tetto     oppure     bash $0 meteo"
    exit 1
fi
FILE_REGOLE="/etc/udev/rules.d/99-osservatorio-$NOME.rules"

elenco_porte() {
    ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | sort || true
}

echo
echo "=== Nome fisso per l'Arduino del $NOME ==="
echo
echo "1) SCOLLEGA ora il cavo USB dell'Arduino del $NOME dal PC."
read -r -p "   Fatto? Premi INVIO... " _
PRIMA=$(elenco_porte)

echo
echo "2) RICOLLEGA ora il cavo USB dell'Arduino del $NOME."
echo "   Usa sempre la stessa presa USB del PC (scrivila su un'etichetta)."
read -r -p "   Fatto? Premi INVIO... " _
sleep 3
DOPO=$(elenco_porte)

NUOVA=$(comm -13 <(echo "$PRIMA") <(echo "$DOPO") | head -n 1)
if [ -z "$NUOVA" ]; then
    echo
    echo "ERRORE: non vedo nessuna porta nuova. Controlla il cavo (alcuni cavi"
    echo "USB portano solo corrente e non i dati) e riprova."
    exit 1
fi
echo
echo "Trovato: $NUOVA"

# Legge le caratteristiche del dispositivo
PROPRIETA=$(udevadm info --query=property --name="$NUOVA")
leggi() { echo "$PROPRIETA" | grep "^$1=" | head -n 1 | cut -d= -f2-; }
VENDOR=$(leggi ID_VENDOR_ID)
PRODUCT=$(leggi ID_MODEL_ID)
SERIALE=$(leggi ID_SERIAL_SHORT)
PERCORSO=$(leggi ID_PATH)

echo "  Produttore (idVendor): $VENDOR"
echo "  Prodotto  (idProduct): $PRODUCT"
echo "  Numero di serie:       ${SERIALE:-(nessuno)}"

# Un numero di serie identifica quell'Arduino in qualunque presa sia collegato.
# Gli Arduino originali ce l'hanno; molti compatibili con chip CH340 no:
# in quel caso si usa la posizione della presa USB.
if [ -n "$SERIALE" ]; then
    REGOLA="SUBSYSTEM==\"tty\", ENV{ID_VENDOR_ID}==\"$VENDOR\", ENV{ID_MODEL_ID}==\"$PRODUCT\", ENV{ID_SERIAL_SHORT}==\"$SERIALE\", SYMLINK+=\"osservatorio-$NOME\", MODE=\"0660\", GROUP=\"dialout\""
    echo
    echo "Questo Arduino ha un numero di serie: funzionera' in QUALSIASI presa USB."
else
    if [ -z "$PERCORSO" ]; then
        echo "ERRORE: non riesco a leggere la posizione della presa USB."
        exit 1
    fi
    REGOLA="SUBSYSTEM==\"tty\", ENV{ID_PATH}==\"$PERCORSO\", SYMLINK+=\"osservatorio-$NOME\", MODE=\"0660\", GROUP=\"dialout\""
    echo
    echo "ATTENZIONE: questo Arduino non ha un numero di serie."
    echo "Il nome fisso funzionera' SOLO in questa presa USB del PC:"
    echo "segna con un'etichetta la presa e il cavo."
fi

echo
echo "Scrivo la regola in $FILE_REGOLE (serve la password di amministratore):"
echo "  $REGOLA"
echo "# Creato da crea_regola_udev.sh il $(date)" | sudo tee "$FILE_REGOLE" > /dev/null
echo "$REGOLA" | sudo tee -a "$FILE_REGOLE" > /dev/null
sudo udevadm control --reload-rules
sudo udevadm trigger --subsystem-match=tty
sleep 2

if [ -e "/dev/osservatorio-$NOME" ]; then
    echo
    echo "FATTO! L'Arduino del $NOME ora si chiama /dev/osservatorio-$NOME"
    ls -l "/dev/osservatorio-$NOME"
else
    echo
    echo "La regola e' stata scritta ma /dev/osservatorio-$NOME non compare ancora."
    echo "Scollega e ricollega l'Arduino; se ancora non compare, riavvia il PC."
fi

#!/bin/bash
# =============================================================================
#  installa.sh  -  Installa sul PC Linux Mint la parte software dell'osservatorio
# =============================================================================
#
#  Cosa fa (e te lo dice passo per passo):
#    1. installa i pacchetti necessari (python3-serial, libnotify-bin);
#    2. controlla che ci siano i driver INDI "RollOff ino" e "Weather Safety Proxy";
#    3. ti aggiunge al gruppo "dialout" (permesso di usare le porte USB);
#    4. crea il file di configurazione ~/.config/osservatorio/meteo.ini
#       (solo se non esiste gia': non sovrascrive mai le tue soglie);
#    5. installa e avvia il servizio "osservatorio-meteo";
#    6. fa una prova finale.
#
#  Uso:   bash strumenti/installa.sh
#  Si puo' rieseguire senza problemi (per esempio dopo un aggiornamento).
# =============================================================================

set -e

REPO="$(cd "$(dirname "$0")/.." && pwd)"
CONFIG_DIR="$HOME/.config/osservatorio"
CONFIG="$CONFIG_DIR/meteo.ini"
SERVIZIO_DIR="$HOME/.config/systemd/user"
SERVIZIO="$SERVIZIO_DIR/osservatorio-meteo.service"

titolo() { echo; echo "=== $1 ==="; }

if [ "$(id -u)" = "0" ]; then
    echo "Non eseguire questo script con sudo: lancialo come utente normale."
    echo "Ti chiedera' la password solo quando serve."
    exit 1
fi

echo "Repository: $REPO"

titolo "1/6  Pacchetti necessari"
DA_INSTALLARE=""
python3 -c "import serial" 2>/dev/null || DA_INSTALLARE="$DA_INSTALLARE python3-serial"
command -v notify-send >/dev/null || DA_INSTALLARE="$DA_INSTALLARE libnotify-bin"
if [ -n "$DA_INSTALLARE" ]; then
    echo "Installo:$DA_INSTALLARE (ti verra' chiesta la password)"
    sudo apt-get update
    sudo apt-get install -y $DA_INSTALLARE
else
    echo "Gia' presenti."
fi

titolo "2/6  Driver INDI"
MANCANO=0
for driver in indi_rolloffino indi_weather_safety_proxy; do
    if command -v "$driver" >/dev/null; then
        echo "  OK  $driver"
    else
        echo "  MANCA  $driver"
        MANCANO=1
    fi
done
if [ "$MANCANO" = "1" ]; then
    echo
    echo "Mancano dei driver INDI. Di solito si installano cosi':"
    echo "    sudo apt install indi-rolloffino indi-bin"
    echo "(richiede il repository di INDI 'ppa:mutlaqja/ppa', lo stesso da cui"
    echo " hai installato KStars; vedi il README, sezione 'Installazione', passo 2)."
    read -r -p "Vuoi che provi a installarli adesso? [s/N] " RISPOSTA
    if [ "$RISPOSTA" = "s" ] || [ "$RISPOSTA" = "S" ]; then
        sudo apt-get install -y indi-rolloffino indi-bin || \
            echo "Installazione non riuscita: segui il README e poi rilancia questo script."
    fi
fi

titolo "3/6  Permesso di usare le porte USB (gruppo dialout)"
if id -nG "$USER" | grep -qw dialout; then
    echo "Fai gia' parte del gruppo dialout."
else
    sudo usermod -a -G dialout "$USER"
    echo "Aggiunto. IMPORTANTE: esci dalla sessione e rientra (o riavvia il PC)"
    echo "perche' il permesso diventi attivo."
fi

titolo "4/6  File di configurazione"
mkdir -p "$CONFIG_DIR"
if [ -f "$CONFIG" ]; then
    echo "Esiste gia' $CONFIG: lo lascio com'e'."
else
    cp "$REPO/meteo/meteo.ini.esempio" "$CONFIG"
    echo "Creato $CONFIG (in modalita' TARATURA)."
fi

titolo "5/6  Servizio meteo (parte da solo all'accensione del PC)"
mkdir -p "$SERVIZIO_DIR"
sed "s#@CARTELLA_REPOSITORY@#$REPO#g" "$REPO/systemd/osservatorio-meteo.service" > "$SERVIZIO"
chmod +x "$REPO/meteo/weather_status.py" "$REPO/meteo/meteo_servizio.py"
systemctl --user daemon-reload
systemctl --user enable osservatorio-meteo.service
systemctl --user restart osservatorio-meteo.service
# Fa partire il servizio anche prima che tu entri nel desktop (dopo un blackout).
sudo loginctl enable-linger "$USER" || true
sleep 4
systemctl --user --no-pager status osservatorio-meteo.service | head -n 5 || true

titolo "6/6  Prova finale"
echo "Risposta che Ekos ricevera' adesso dalla stazione meteo:"
"$REPO/meteo/weather_status.py"
echo
echo "(In modalita' taratura, o con l'Arduino meteo non ancora collegato, e' normale"
echo " leggere \"open_ok\": 0, cioe' NON SICURO.)"

echo
echo "================================================================="
echo " INSTALLAZIONE COMPLETATA"
echo "================================================================="
echo " In Ekos, driver 'Weather Safety Proxy', scheda Options, scrivi"
echo " come 'Weather safety script' questo percorso:"
echo
echo "     $REPO/meteo/weather_status.py"
echo
echo " Comandi utili:"
echo "   vedere lo stato dal vivo:   watch -n 2 cat ~/osservatorio-meteo/stato_meteo.json"
echo "   vedere il registro eventi:  tail -f ~/osservatorio-meteo/meteo.log"
echo "   riavviare il servizio:      systemctl --user restart osservatorio-meteo"
echo "   modificare le soglie:       xed $CONFIG"
echo "================================================================="

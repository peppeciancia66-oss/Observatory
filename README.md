# Osservatorio: automazione del tetto roll-off e stazione meteo

Questo repository contiene tutto il necessario per comandare il **tetto
roll-off** e la **stazione meteo fai-da-te** dell'osservatorio da
**KStars/Ekos** (INDI) su **Linux Mint**.

- Il **tetto** è comandato da un **Arduino Uno** tramite il driver INDI già
  esistente **"RollOff ino"**. L'Arduino "preme" i tasti APRI / CHIUDI / STOP
  della scheda del cancello per mezzo di relè e legge i finecorsa, il sensore
  di parcheggio della montatura, l'emergenza e la pioggia.
- La **stazione meteo** (sensore a infrarossi MLX90614, anemometro, sensore
  di pioggia) è letta da un **Arduino Nano**. Un programma Python sul PC
  decide se le condizioni sono sicure e lo comunica a Ekos tramite il driver
  **"Weather Safety Proxy"**.

## Stato del progetto

| Parte | Stato |
|---|---|
| Sketch Arduino del tetto, con tutte le sicurezze | ✅ Pronto e provato con un simulatore (61 verifiche automatiche) |
| Sketch Arduino della stazione meteo | ✅ Pronto (compila per Arduino Nano) |
| Programma meteo + collegamento a Ekos | ✅ Pronto e provato (48 test automatici) |
| Installazione, collaudo, taratura | ✅ Documentati |
| **Collegamento tra relè e scheda del cancello** | ⏳ **In attesa della foto della scheda** (vedi [docs/04](docs/04-scheda-cancello.md)) |

## Documenti (leggili in quest'ordine)

1. [Architettura: cosa è collegato a cosa, e cosa succede se il PC si blocca](docs/01-architettura.md)
2. [Lista della spesa](docs/02-lista-spesa.md)
3. [Schema dei collegamenti](docs/03-collegamenti.md)
4. [La scheda del cancello: come verificarla e quali relè usare](docs/04-scheda-cancello.md) ⏳
5. [Protezioni hardware e dove serve l'elettricista](docs/05-protezioni-hardware.md)
6. [Montaggio della stazione meteo e protezione dalle intemperie](docs/06-montaggio-stazione-meteo.md)
7. [Procedura di collaudo in sicurezza](docs/07-collaudo.md)
8. [Taratura della stazione meteo](docs/08-taratura-meteo.md)

## Contenuto del repository

```
arduino/tetto/tetto.ino          programma dell'Arduino Uno (tetto)
arduino/tetto/test/              prove automatiche del programma del tetto (simulatore)
arduino/meteo/meteo.ino          programma dell'Arduino Nano (stazione meteo)
meteo/meteo_servizio.py          programma meteo (gira sempre, in sottofondo)
meteo/weather_status.py          script chiamato da Ekos (Weather Safety Proxy)
meteo/meteo.ini.esempio          configurazione con tutte le soglie
meteo/osservatorio_meteo/        il codice del programma meteo
meteo/tests/                     test automatici del programma meteo
strumenti/installa.sh            installa tutto sul PC
strumenti/crea_regola_udev.sh    dà un nome fisso a ciascun Arduino
strumenti/prova_tetto.py         prova del tetto senza Ekos (collaudo)
strumenti/riepilogo_taratura.py  riassunto dei dati registrati
systemd/                         servizio che avvia il programma meteo da solo
docs/                            tutta la documentazione
```

---

# Installazione passo passo

> **Come leggere le istruzioni.** Le righe in un riquadro grigio come
> questa:
>
> ```bash
> echo ciao
> ```
>
> sono **comandi** da scrivere nel **Terminale**. Copiale, incollale nel
> Terminale (tasto destro → Incolla, oppure Ctrl+Maiusc+V) e premi **Invio**.
> Quando un comando inizia con `sudo` ti verrà chiesta la **tua** password:
> mentre la scrivi non compare nulla sullo schermo, è normale.

## Passo 1 — Aprire il Terminale

Menu in basso a sinistra → scrivi **Terminale** → clic sull'icona. Si apre
una finestra con il cursore che lampeggia.

## Passo 2 — Controllare KStars e i driver INDI

Il PC ha già KStars/Ekos. Verifichiamo che ci siano i due driver che ci
servono:

```bash
command -v indi_rolloffino indi_weather_safety_proxy
```

- Se compaiono **due righe** (per esempio `/usr/bin/indi_rolloffino`), passa
  al passo 3.
- Se ne manca una, installa (o aggiorna) INDI dal repository ufficiale:

```bash
sudo add-apt-repository ppa:mutlaqja/ppa
```

```bash
sudo apt update
```

```bash
sudo apt install indi-full kstars-bleeding
```

  Poi ripeti il controllo. *(Se uno di questi comandi dà errore, copiami il
  messaggio: dipende dalla versione di Linux Mint, che mi comunicherai.)*

## Passo 3 — Scaricare questo repository

```bash
sudo apt install git
```

```bash
git clone https://github.com/peppeciancia66-oss/Observatory.git ~/observatory
```

Ora c'è una cartella **observatory** nella tua cartella personale. Tutti i
comandi di queste istruzioni presuppongono che sia lì.

## Passo 4 — Installare il software

```bash
bash ~/observatory/strumenti/installa.sh
```

Lo script spiega ogni passaggio. Alla fine scrive il percorso da copiare in
Ekos (passo 8). Se dice che il tuo utente è stato aggiunto al gruppo
**dialout**, **esci dalla sessione e rientra** (o riavvia il PC) prima di continuare.

## Passo 5 — Installare l'IDE di Arduino

L'**IDE di Arduino** è il programma che serve a caricare gli sketch
(i programmi) sugli Arduino.

1. Menu → **Gestore applicazioni** → cerca **Arduino IDE** → scegli la
   versione **2.x** → **Installa**.
2. In alternativa, scaricalo da <https://www.arduino.cc/en/software>
   (versione Linux).

## Passo 6 — Caricare gli sketch sugli Arduino

Carica **un Arduino alla volta**, con l'altro scollegato, così non ti
confondi.

**Arduino del tetto (Uno):**

1. Collega l'Arduino Uno al PC con il cavo USB.
2. Apri l'IDE di Arduino → **File → Apri** →
   `observatory/arduino/tetto/tetto.ino`.
3. In alto, nel menu a tendina della scheda, scegli **Arduino Uno** e la
   porta (di solito `/dev/ttyACM0`).
4. Premi il pulsante con la **freccia verso destra** (Carica). In basso deve
   comparire "Caricamento completato" (o "Done uploading").

**Arduino della stazione meteo (Nano):**

1. Scollega l'Uno e collega il Nano.
2. **File → Apri** → `observatory/arduino/meteo/meteo.ino`.
3. Scheda: **Arduino Nano**. Porta: di solito `/dev/ttyUSB0`.
4. Carica. Se dà un errore di caricamento con un Nano compatibile, vai su
   **Strumenti → Processore** → **ATmega328P (Old Bootloader)** e riprova.
5. Per controllare: **Strumenti → Monitor seriale**, velocità **9600**. Ogni
   2 secondi deve comparire una riga che inizia con `{"id":"oss-meteo"`.
   Chiudi il monitor seriale prima di andare avanti.

> Non servono librerie aggiuntive: entrambi gli sketch usano solo ciò che è
> già incluso nell'IDE.

## Passo 7 — Dare un nome fisso ai due Arduino

Chiudi l'IDE di Arduino. Poi, per il tetto:

```bash
bash ~/observatory/strumenti/crea_regola_udev.sh tetto
```

e per la stazione meteo:

```bash
bash ~/observatory/strumenti/crea_regola_udev.sh meteo
```

Segui le istruzioni a schermo (scollega/ricollega). Alla fine esistono
`/dev/osservatorio-tetto` e `/dev/osservatorio-meteo`. Riavvia il servizio
meteo perché usi il nuovo nome:

```bash
systemctl --user restart osservatorio-meteo
```

## Passo 8 — Configurare Ekos

> I nomi dei menu possono cambiare leggermente da una versione di KStars
> all'altra. Se non trovi qualcosa, mandami una schermata.

**8.1 Il profilo**

1. Apri **KStars** → menu **Strumenti → Ekos**.
2. Accanto al profilo che usi, premi la **matita** (modifica profilo).
3. Nel campo **Dome** scegli **RollOff ino**.
4. Nel campo **Weather** scegli **Weather Safety Proxy**.
5. **Salva**. Avvia il profilo con il pulsante **Play** ▶.

**8.2 Il tetto (driver RollOff ino)**

1. Si apre il **pannello di controllo INDI**. Clicca sulla scheda
   **RollOff ino**.
2. Scheda **Connection**:
   - **Port** = `/dev/osservatorio-tetto`
   - **Baud Rate** = `38400`
   - premi **Connect**.
3. Scheda **Options**:
   - **Mount Policy** = **Mount locks** (il tetto non si chiude se la
     montatura non è parcheggiata);
   - **Roof Movement → Timeout in Seconds** = tempo massimo dell'Arduino
     + 10 (inizialmente **90**; lo aggiornerai al collaudo, fase C.3);
   - in fondo premi **Configuration → Save**.

**8.3 Il meteo (driver Weather Safety Proxy)**

1. Scheda **Weather Safety Proxy** → **Options**:
   - **Script or url** = **Use script**;
   - **Weather safety script** = il percorso indicato da `installa.sh`, cioè
     `/home/TUO_NOME_UTENTE/observatory/meteo/weather_status.py`. Per
     scoprire il tuo nome utente scrivi `whoami` nel Terminale;
   - **Configuration → Save**.
2. Scheda **Main Control**: il periodo di aggiornamento (**Period**) va bene
   a **60** secondi.
3. Lo stato **Weather Safety** sarà rosso finché sei in modalità taratura.
   È corretto: vedi il campo "Weather Safety Reasons".

**8.4 La chiusura automatica per il meteo (resta disattivata)**

Nel modulo **Observatory** di Ekos c'è una sezione **Weather** con le azioni
da compiere quando il meteo diventa "Warning" o "Alert" (per esempio
parcheggiare la montatura e chiudere il tetto).

👉 **Lascia queste azioni tutte disattivate** finché non hai finito collaudo
e taratura (docs/07 fase D, docs/08). Quando le attiverai, l'ordine giusto è:
prima **parcheggiare la montatura**, poi **chiudere il tetto**.

## Passo 9 — Collaudo

Segui [docs/07-collaudo.md](docs/07-collaudo.md) dall'inizio. **Non saltare
nessuna fase.**

---

# Uso quotidiano

- **Aprire:** parcheggia la montatura → Ekos → **Observatory** → **Open**
  (oppure lascia che lo faccia lo Scheduler).
- **Chiudere:** parcheggia la montatura → **Close**.
- **Tetto bloccato (luce rossa "Roof Lock").** Il motivo può essere:
  montatura non parcheggiata, emergenza premuta, finecorsa incoerenti, oppure
  un errore durante un movimento (tempo scaduto, motore che non parte, PC
  muto...).
  1. **Vai a controllare il tetto di persona.**
  2. Per sapere il motivo, disconnetti il tetto in Ekos ed esegui
     `python3 ~/observatory/strumenti/prova_tetto.py`, scelta 5.
  3. Poi, in Ekos, **Disconnect** e **Connect** del tetto: questo "riconosce"
     l'errore e sblocca il tetto.
- **Vedere il meteo dal vivo:**

  ```bash
  watch -n 2 cat ~/osservatorio-meteo/stato_meteo.json
  ```

- **Cambiare una soglia:** apri `~/.config/osservatorio/meteo.ini` con
  l'editor di testo, cambia il numero e salva. Le nuove soglie valgono entro
  5 secondi.
- **Registro degli eventi:** `~/osservatorio-meteo/meteo.log`.

# Aggiornare il software

```bash
cd ~/observatory && git pull
```

```bash
bash ~/observatory/strumenti/installa.sh
```

Se è cambiato uno sketch (`.ino`), ricaricalo sull'Arduino (passo 6).
**Attenzione:** caricando di nuovo `tetto.ino` si perdono le modifiche fatte
nella sezione CONFIGURAZIONE (per esempio `TEMPO_MAX_MOVIMENTO_S`). Annotale
prima.

# Per chi vuole verificare il codice

```bash
cd ~/observatory/meteo && python3 -m pytest tests
```

```bash
bash ~/observatory/arduino/tetto/test/esegui_prove.sh
```

Il primo comando richiede il pacchetto `python3-pytest`, il secondo `g++`.

# Licenza

MIT: vedi [LICENSE](LICENSE).

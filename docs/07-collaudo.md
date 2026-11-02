# 7. Procedura di collaudo in sicurezza

Il collaudo va fatto **in quest'ordine**, senza saltare fasi. Ogni fase ha
delle caselle da spuntare: stampa questa pagina e tienila sul banco. Se un
risultato non è quello atteso, **fermati** e scrivimi cosa hai visto.

| Fase | Cosa si prova | Motore |
|---|---|---|
| 0 | Il software, senza hardware | — |
| A | Arduino, relè e sensori sul banco | **scollegato** dalla scheda |
| B | Comandi verso la scheda | collegato ma **sbloccato** (gira a vuoto, il tetto non si muove) |
| C | Il tetto vero | **in movimento, sotto sorveglianza** |
| D | Stazione meteo e Ekos | — |
| E | (facoltativa) Chiusura autonoma per pioggia | in movimento, sotto sorveglianza |

---

## Fase 0 — Software senza hardware (al PC, in qualsiasi momento)

**0.1 Tetto simulato in Ekos**

- [ ] In Ekos crea un profilo di prova con, come "Dome", il driver
      **RollOff Simulator** (si trova tra le cupole, "Domes").
- [ ] Avvia il profilo. Nel modulo **Observatory** premi Open e poi Close:
      il tetto simulato si apre e si chiude.
- [ ] Così impari dove sono i pulsanti senza rischi.

**0.2 Arduino del tetto virtuale (facoltativo, richiede `sudo apt install g++`)**

Lo sketch vero gira sul PC con un tetto simulato. Serve a imparare a usare
`prova_tetto.py` prima di avere l'hardware.

1. In un Terminale:

   ```bash
   bash ~/observatory/arduino/tetto/test/esegui_tetto_virtuale.sh
   ```

2. In un secondo Terminale:

   ```bash
   python3 ~/observatory/strumenti/prova_tetto.py /tmp/tetto-virtuale
   ```

3. Spunta le verifiche:

- [ ] Scelta 2 (APRI): lo stato passa a "IN APERTURA" e poi ad "APERTO".
- [ ] Nel primo Terminale scrivi `m` e premi Invio: la montatura risulta non
      parcheggiata e il movimento è "BLOCCATO".

**0.3 Meteo simulato**

- [ ] In `~/.config/osservatorio/meteo.ini` metti `porta = simulazione` e
      `modalita = normale`.
- [ ] `systemctl --user restart osservatorio-meteo`
- [ ] Esegui `watch -n 2 cat ~/osservatorio-meteo/stato_meteo.json`:
      dopo `attesa_ritorno_sicuro_min` minuti (15 di default) deve comparire
      `"sicuro": true`.
- [ ] Nella sezione `[simulazione]` metti `pioggia = si` e salva. Entro
      pochi secondi: `"sicuro": false`, motivo "pioggia".
- [ ] Metti `mlx_funzionante = no`: compare un **GUASTO** e arriva una
      notifica sul desktop.
- [ ] Rimetti tutto com'era, con `porta = /dev/osservatorio-meteo` e
      `modalita = taratura`, e riavvia il servizio.

---

## Fase A — Al banco, motore SCOLLEGATO

Sul tavolo ci sono solo: PC, Arduino Uno, modulo relè, i finecorsa (in mano),
i contatti magnetici, il pulsante di emergenza (anche solo il contatto
ausiliario, o un filo che simula il contatto) e il tester. **Nessun filo va
alla scheda del cancello.**

**A.1 Caricare lo sketch** (vedi README, "Caricare gli sketch sugli Arduino")

- [ ] Sketch `tetto.ino` caricato senza errori.

**A.2 Relè a riposo all'accensione**

- [ ] Scollega e ricollega l'Arduino: **nessun relè deve scattare**
      all'accensione. I LED del modulo relè devono restare spenti.
- [ ] Se invece i relè restano tutti attivi, il modulo è "attivo alto": in
      `configurazione_scheda.h` metti `RELE_ATTIVO_BASSO` a `0` e ricarica lo sketch.

**A.3 Lettura dei sensori**

1. Chiudi Ekos.
2. Esegui:

   ```bash
   python3 ~/observatory/strumenti/prova_tetto.py
   ```

3. Spunta le verifiche:

- [ ] Con nessun sensore collegato: "Movimento BLOCCATO: SI". È corretto:
      senza il sensore della montatura il tetto non si deve muovere. Il LED
      dell'Arduino lampeggia lentamente.
- [ ] Collega i due contatti della montatura in serie e avvicina i magneti:
      il blocco sparisce solo quando **entrambi** i magneti sono vicini.
- [ ] Collega il contatto di emergenza (chiuso): il blocco sparisce.
      Premi il fungo: "Movimento BLOCCATO: SI".
- [ ] Premi a mano il finecorsa CHIUSO: "TETTO CHIUSO: SI". Rilascialo: "no".
- [ ] Premi a mano il finecorsa APERTO: "TETTO APERTO: SI".
- [ ] Premi **entrambi** insieme: "Movimento BLOCCATO: SI" e "STATO DEL
      TETTO: FINECORSA INCOERENTI".
- [ ] Con nessun finecorsa premuto: "STATO DEL TETTO: FERMO A META'".
- [ ] Togli un filo a un finecorsa: deve risultare "no", mai "SI".

**A.4 I relè**

1. Metti il tester in modalità "continuità" (bip) sui morsetti COM e NO
   del relè 1.
2. Tieni premuto il finecorsa CHIUSO (tetto "chiuso") e i magneti in
   posizione di parcheggio.

- [ ] Scelta **2 (APRI)**: si sente il "clic" del relè 1 e il tester fa
      bip per circa mezzo secondo. Gli altri relè non scattano.
- [ ] **Senza rilasciare il finecorsa CHIUSO**, aspetta 8 secondi: si
      deve sentire il clic del relè 3 (STOP). Alla scelta 5 compare il
      motivo "il tetto non e' partito". È la protezione
      "motore che non parte".
- [ ] Scelta 6 (azzera errore). Ora rilascia il finecorsa CHIUSO (tetto "a
      metà"), scegli 2 (APRI) e **non toccare nulla**: dopo
      `TEMPO_MAX_MOVIMENTO_S` secondi (45 di default) scatta lo STOP e il
      motivo è "tempo massimo di movimento superato".
- [ ] Scelta 6. Scegli 3 (CHIUDI), poi dopo 5 secondi allontana un
      magnete della montatura: STOP immediato, motivo "montatura uscita dal
      parcheggio".
- [ ] Prova del "PC muto". Collega l'alimentatore 12 V all'Arduino. Scelta
      6, poi scelta 2 (APRI), poi **stacca il cavo USB**. Il programma
      segnala un errore di comunicazione: è normale. Entro 15 secondi deve
      scattare il relè STOP (lo senti) e il LED dell'Arduino lampeggia
      veloce. Ricollega la USB e rilancia il programma.
- [ ] Con il tetto "a metà" e i magneti lontani, scegli 2 (APRI): la
      risposta è un rifiuto (NAK) e **nessun relè scatta**.

**A.5 Sensore di pioggia**

- [ ] Col tester trova, sull'RG-11 alimentato a 12 V, il contatto che è
      **chiuso quando è asciutto** e si **apre** se spruzzi acqua sulla
      cupola.
- [ ] Collega il relè di appoggio come in `03-collegamenti.md` (3.4).
      Spruzza acqua: con `prova_tetto.py` "Pioggia: SI". Asciuga la cupola
      dopo qualche minuto: "no".
- [ ] Togli l'alimentazione a 12 V del relè di appoggio: "Pioggia: SI"
      (perché un guasto deve valere come pioggia).

**A.6 Ekos con il tetto vero (ancora scollegato dalla scheda)**

- [ ] Configura Ekos come nel README ("Configurare Ekos").
- [ ] Connetti: nel pannello INDI del tetto si accendono le luci di stato
      ("Roof Status").
- [ ] Con i magneti lontani: luce rossa "Roof Lock". Il pulsante Open non
      fa scattare nessun relè e nel registro compare "Roof is externally
      locked".
- [ ] Con i magneti vicini e il finecorsa CHIUSO premuto: premi **Open**,
      scatta il relè 1. Rilascia il CHIUSO e premi l'APERTO: Ekos mostra
      il tetto aperto.

---

## Fase B — Collegamento alla scheda, motore SBLOCCATO ⏳

> Si fa **solo dopo** che:
>
> - i controlli di [10-controlli-senza-corrente.md](10-controlli-senza-corrente.md)
>   e le misure E1-E3 dell'elettricista sono fatti;
> - io ho aggiornato `configurazione_scheda.h` e `docs/03` con la strada
>   scelta (A o B).
>
> Le voci qui sotto valgono per entrambe le strade.

**B.1 Preparazione (corrente tolta, con l'elettricista)**

- [ ] I fili dei relè sono collegati come indicato in `docs/03` (strada A o
      B), con il cavo separato dai fili di corrente e motore.
- [ ] Il motore è **sbloccato** con la chiave di sblocco manuale: il motore
      può girare, ma il tetto non si muove (H21). Se non c'è un modo di
      sbloccarlo, l'elettricista scollega temporaneamente il motore e si
      osservano solo i LED e i relè della scheda.
- [ ] Telescopio parcheggiato, nessuno vicino al binario.

**B.2 I tre comandi (corrente ridata dall'elettricista)**

Con `prova_tetto.py`, tenendo premuto a mano il finecorsa CHIUSO, come se il
tetto fosse chiuso:

- [ ] Scelta 2 (APRI): il motore gira **nel verso di apertura**. Dopo 8
      secondi l'Arduino preme STOP, perché il tetto non si muove e il
      finecorsa resta premuto: il motore deve fermarsi. Così si provano
      insieme APRI, STOP e la protezione "il tetto non è partito" (H6, H9,
      H10, H15).
- [ ] Scelta 6, poi scelta 3 (CHIUDI) tenendo premuto il finecorsa APERTO:
      il motore gira nel verso **opposto**, poi STOP dopo 8 secondi.
- [ ] Se un comando ogni tanto non viene ricevuto, allunga `IMPULSO_MS` in
      `configurazione_scheda.h` (per esempio a 1000) e riprova (H15).
- [ ] Premi **STOP** (scelta 4) a motore fermo: il motore **non** deve
      partire (H9).

**B.3 Richiusura automatica (la più importante)**

- [ ] Dai APRI e lascia che l'Arduino fermi tutto. Poi **aspetta 60
      secondi senza fare nulla**: il motore **non deve ripartire da solo**
      (H12, H13). Se riparte, emergenza: la richiusura automatica è attiva,
      e la fase C **non si fa** finché non è disattivata.

**B.4 Pulsante di emergenza**

- [ ] Durante un comando APRI premi il fungo: il motore si ferma subito e
      l'Arduino mostra "Movimento BLOCCATO".

## Fase C — Il tetto si muove (SOTTO SORVEGLIANZA)

**Regole di sicurezza per tutta la fase C:**

- Ci sono **due persone**: una al PC, l'altra **con la mano vicino al
  pulsante di emergenza** e lo sguardo sul tetto.
- Nessuno sta sotto il tetto o sul binario.
- Il telescopio è **parcheggiato**, e la prima volta anche coperto o
  smontato.
- Si procede **un passo alla volta**.

**C.1 Finecorsa meccanici**

- [ ] Con il motore sbloccato, sposta il tetto **a mano** fino in fondo,
      da tutte e due le parti. Il nostro finecorsa APERTO (o CHIUSO) deve
      scattare **prima** che il tetto arrivi ai fermi meccanici e ai
      finecorsa della scheda. Lo controlli con `prova_tetto.py`.
- [ ] Ricontrolla l'ingombro: con il telescopio in parcheggio, il tetto
      passa con almeno 10 cm di margine.

**C.2 Primo movimento breve**

- [ ] Riblocca il motore. Tetto chiuso.
- [ ] Da `prova_tetto.py`: scelta 2 (APRI), e dopo 3-4 secondi scelta 4
      (STOP). Il tetto deve partire **nel verso giusto** e fermarsi.
- [ ] Se va nel verso sbagliato, **emergenza**. Poi, a corrente staccata,
      scambia i fili dei relè 1 e 2.

**C.3 Misura della corsa e tempo massimo**

- [ ] Apri tutto e cronometra: tempo di apertura = ____ s.
- [ ] Chiudi tutto e cronometra: tempo di chiusura = ____ s.
- [ ] In `configurazione_scheda.h` metti `TEMPO_MAX_MOVIMENTO_S` = tempo più
      lungo × 1,3, arrotondato. Ricarica lo sketch.
- [ ] In Ekos, scheda Options del tetto, imposta "Roof Timeout" a
      `TEMPO_MAX_MOVIMENTO_S` + 10.

**C.4 Prove di arresto (con il tetto in movimento)**

- [ ] **Emergenza:** durante l'apertura premi il fungo. Il tetto si ferma
      subito. In Ekos compare "Roof Lock". Sblocca il fungo, riconnetti il
      tetto in Ekos e richiudi.
- [ ] **Abort da Ekos:** durante l'apertura premi "Abort" nel modulo
      Observatory o nel pannello INDI: il tetto si ferma.
- [ ] **PC muto:** durante l'apertura stacca il cavo USB dell'Arduino del
      tetto (l'Arduino è alimentato a 12 V): entro 15 s il tetto si ferma.
- [ ] **Montatura:** durante l'apertura allontana a mano un magnete del
      sensore di parcheggio: il tetto si ferma subito.
- [ ] **Movimento non comandato:**
  1. Tetto aperto. Allontana un magnete del sensore di parcheggio (come se
     si stesse osservando) e aspetta 10 secondi.
  2. Premi **close** sul telecomando: l'Arduino deve premere STOP entro un
     paio di secondi.
  3. Con `prova_tetto.py`, scelta 5: "avviso: movimento non comandato…".
- [ ] (Se hai montato il sensore di movimento) **Fermo a metà:** durante
      l'apertura premi Stop sul **telecomando**. L'Arduino se ne accorge
      dopo pochi secondi: "il tetto si e' fermato a meta'".
- [ ] **Ostacolo** (protezione della scheda): metti una scatola di cartone
      vuota sul percorso. La scheda deve fermare il motore al contatto. Se
      non lo fa, regola la sensibilità seguendo il manuale.

**C.5 Ciclo completo da Ekos**

- [ ] Montatura parcheggiata. Da Ekos, modulo Observatory: **Open**. Il tetto
      si apre completamente e Ekos mostra "aperto".
- [ ] Togli la montatura dal parcheggio. Prova **Close**: Ekos lo **rifiuta**.
- [ ] Parcheggia la montatura da Ekos. **Close**: il tetto si chiude.
- [ ] Ripeti il ciclo completo **10 volte** di seguito, sempre sorvegliando.
      Annota ogni anomalia.

---

**C.6 Dopo un'interruzione di corrente (ipotesi H19)**

- [ ] Tetto **a metà corsa**. L'elettricista toglie e ridà la corrente alla
      scheda del cancello (l'Arduino resta acceso con il suo alimentatore).
- [ ] Dai **CHIUDI** da `prova_tetto.py`, con la mano sul fungo di
      emergenza. Annota in che verso parte il tetto: ________.
- [ ] Se parte in **apertura** invece che in chiusura, la scheda, dopo un
      blackout, "dimentica" la posizione. Annotalo e scrivimelo. Da quel
      momento la regola è: **dopo ogni blackout, la prima manovra si fa
      sorvegliando**.

## Fase D — Stazione meteo e Ekos

- [ ] Arduino Nano caricato con `meteo.ino`, regola udev creata
      (`/dev/osservatorio-meteo`).
- [ ] Servizio avviato. Con `watch -n 2 cat ~/osservatorio-meteo/stato_meteo.json`
      vedi temperature sensate (aria simile a un termometro, cielo molto
      più fredda di notte se è sereno).
- [ ] Gira le coppette: il vento sale. Spruzza acqua sul sensore di pioggia:
      pioggia "true".
- [ ] Stacca il cavo dell'MLX90614: entro 10 secondi notifica di
      **GUASTO**.
- [ ] Stacca il cavo USB del Nano: entro 30 secondi notifica di **GUASTO**
      ("non invia dati").
- [ ] In Ekos, driver **Weather Safety Proxy**: in modalità taratura lo stato
      è "Alert" (rosso), con il motivo "MODALITA' TARATURA…". È corretto.
- [ ] Ferma il servizio (`systemctl --user stop osservatorio-meteo`): Ekos
      mostra "programma meteo fermato". Riavvialo.
- [ ] Fai la **taratura** (`08-taratura-meteo.md`) per almeno 2-3 settimane,
      poi passa a `modalita = normale`.

**Solo se decidi di attivare la chiusura automatica per il meteo da Ekos:**

- [ ] Prova prima con il meteo **simulato** (fase 0.3) e il tetto vero sotto
      sorveglianza: `pioggia = si` → Ekos parcheggia la montatura e poi chiude
      il tetto.

---

## Fase E — (Facoltativa) Chiusura autonoma per pioggia dall'Arduino

Si fa solo dopo che **tutte** le fasi precedenti sono andate bene.

- [ ] In `tetto.ino` metti `CHIUSURA_AUTONOMA_PIOGGIA` a `1` e ricarica
      lo sketch.
- [ ] Tetto aperto, montatura parcheggiata, **sorveglianza**. Stacca il cavo
      USB dell'Arduino del tetto (l'Arduino resta acceso con i 12 V). Spruzza
      acqua sul sensore di pioggia e **continua a bagnarlo**. Dopo circa 2
      minuti di "PC muto" + 30 s di pioggia, il tetto si chiude da solo.
- [ ] Ripeti con la montatura **NON** parcheggiata: il tetto **non** si deve
      muovere.
- [ ] Ripeti con il cavo USB collegato ed Ekos connesso: il tetto **non** si
      chiude da solo, perché in questo caso è Ekos a decidere.
- [ ] Ricollega tutto e riconnetti il tetto in Ekos.

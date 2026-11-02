# 10. Controlli da fare con la corrente tolta

## In breve: i 3 controlli più importanti dei prossimi 5 giorni

> Stampa questa facciata. Corrente **tolta** (controllo C1) e **niente si
> sposta**: solo guardare, fotografare e scrivere. Per ogni punto: una foto
> ben illuminata (luce di lato), ravvicinata, controllata ingrandendola sul
> telefono prima di mandarla.

**1. I DIP rossi e la richiusura automatica** (il più importante)

- **Foto:** il blocchetto rosso dei DIP, da vicinissimo, con una luce di
  lato. Si devono leggere la scritta **ON** e i numeri 1-2-3-4.
- **Annota:** per ogni levetta (1, 2, 3, 4), se è dalla parte di ON oppure
  no: 1 ____ 2 ____ 3 ____ 4 ____
- **Foto:** la tabellina stampata vicino ai DIP ("OFF 5", "OFF ON 10",
  "ON ON 20"…), tutta intera.
- **Perché:** i DIP 2 e 3 ("Self closing") comandano la **richiusura
  automatica**. Se fosse accesa, il tetto si richiuderebbe da solo dopo
  ogni apertura: è il rischio più grande (controllo C8).

**2. Le scritte "ope / sto / clo" e i fili collegati oggi**

- **Foto:** il morsetto verde a **3 posizioni**, fotografato **da sinistra**
  e un po' dall'alto, in modo che si leggano le scritte **complete** accanto
  a ogni polo.
- **Annota:** la scritta completa di ogni polo, dall'alto in basso:
  ________ / ________ / ________
- **Annota:** ci sono fili collegati al morsetto verde a 3 posizioni? E alla
  fila "+15V out / GND / Photo / OPN / test"? Per ognuno: colore e dove va.
- **Foto:** il morsetto verde a **2 posizioni** vicino al fusibile, da
  un'angolazione da cui la scritta non sia coperta dal filo dell'antenna.
  **Non toccarlo e non spostare il filo.**
- **Perché:** serve a capire a quali ingressi si collegheranno i relè
  (controlli C3, C4, C5a, C5b).

**3. Cosa chiedere all'elettricista ⚡** (a scheda accesa: solo lui)

Mostragli questa pagina e chiedigli di misurare e scrivere:

- **E1 – Tensioni.** Con il puntale nero sul morsetto **GND** della fila a
  vite, la tensione (continua e alternata) su: +15V out, Photo, OPN, test,
  **ognuno dei 3 poli** del morsetto verde "ope / sto / clo", **ognuno dei 2
  poli** del morsetto verde vicino al fusibile.
- **E2 – Richiusura automatica.** Con il motore **sbloccato** (chiave di
  sblocco, così il tetto non si muove): un comando di apertura col
  telecomando, poi **60 secondi di attesa**. Il motore **non** deve
  ripartire da solo.
- **E3 – Sicurezza.** Messa a terra della carcassa del motore, e fase e
  neutro collegati correttamente.

Quando avrai foto, annotazioni e misure, mandamele: aggiornerò solo
`configurazione_scheda.h`, lo schema dei collegamenti e lo stato delle
ipotesi. **Fino ad allora non si collega nulla alla scheda.**

---

## I controlli completi

Questa lista serve a raccogliere i dati che mancano. Per ogni controllo
trovi **cosa fare**, **quale foto fare o cosa annotare** e **perché serve**
(tra parentesi le ipotesi di [09-ipotesi-da-verificare.md](09-ipotesi-da-verificare.md)).

Stampa questa pagina e scrivi le risposte direttamente sul foglio.

> ⚠️ **Regole per tutti i controlli**
>
> - Si lavora **solo con la corrente tolta**, e la prima volta **con
>   l'elettricista** (controllo C1).
> - **Non si sposta niente:** né DIP, né potenziometro, né selettori, né fili.
>   Oggi si guarda, si fotografa e si annota soltanto.
> - Non si infilano attrezzi metallici nella scheda. Per indicare un
>   morsetto nella foto usa uno stuzzicadenti di legno o di plastica.

## Consigli per le foto

- **Luce.** Usa una lampada o la torcia di un altro telefono, tenuta **di
  lato**: le scritte bianche sulla scheda blu si leggono meglio. Fai ogni
  foto due volte, con e senza flash.
- **Distanza e messa a fuoco.** Avvicinati fino a 10-15 cm e tocca lo
  schermo sul punto da mettere a fuoco. La foto 4 che mi hai mandato era
  scura e sfocata.
- **Controllo prima di mandarla.** Ingrandisci la foto sul telefono: se non
  riesci a leggere le scritte tu, non ci riesco nemmeno io.
- **Numerazione.** Chiama le foto con il numero del controllo (C3, C5…).

---

## C1 — Togliere la corrente in sicurezza ⚡

- **Cosa fare:**
  1. L'elettricista spegne l'interruttore dedicato al cancello.
  2. Ci mette un cartello "NON ACCENDERE – lavori in corso".
  3. Verifica con il suo strumento che sulla scheda **non ci sia tensione**.
  4. Si aspettano **5 minuti** prima di avvicinarsi, perché i condensatori
     si scarichino.
- **Da annotare:** quale interruttore alimenta la scheda (fai una foto del
  quadro con l'interruttore indicato).
- **Perché:** sulla scheda ci sono morsetti a 230 V.

## C2 — La scheda intera, così com'è oggi

- **Foto:**
  - una foto dall'alto della scheda intera, con tutti i fili collegati;
  - una da destra e una da sinistra, inclinate.
- **Da annotare:** quante morsettiere ci sono e di che colore.
- **Perché:** queste foto non sono arrivate. Servono a capire quali
  connettori esistono e dove vanno i fili oggi (H1, H4, H20).

## C3 — La morsettiera a vite "+15V out / GND / Photo / OPN / test"

- **Foto:** ravvicinata e dritta, con le scritte ben leggibili, sia quelle
  sopra sia quelle sotto i morsetti. Se i fili coprono le scritte, fai più
  foto da angolazioni diverse.
- **Da annotare**, per ciascuno dei morsetti da sinistra a destra:
  - la scritta (es. "OPN");
  - se c'è un filo, e di che colore;
  - dove va quel filo.
- **Perché:** confermare la numerazione 1-5 del manuale e qual è il comune
  (H1, H2, H6).

## C4 — Ponticelli sulla morsettiera

- **Da annotare:** c'è un pezzetto di filo che unisce due morsetti (un
  "ponticello")? Tra quali? In particolare tra GND e Photo?
- **Foto:** ravvicinata del ponticello.
- **Perché:** molte schede richiedono un ponticello sulla fotocellula per
  poter chiudere (H7).

## C5 — I due morsetti verdi ⚡ (solo guardare)

> **Aggiornato dopo le foto 6 e 7 del 2 novembre.** Sulla scheda i morsetti
> verdi sono **due**:
>
> - **verde a 3 posizioni**, in basso a sinistra, vicino alla fila
>   "+15V out / GND / Photo / OPN / test". Accanto si leggono solo l'inizio
>   delle scritte "ope…", "sto…", "clo…": il resto è coperto dal morsetto.
>   Nella foto non si vedono fili collegati;
> - **verde a 2 posizioni**, a destra, vicino al fusibile, ai due relè
>   grandi e ai morsetti con i fili blu e marrone (fili di corrente). La
>   scritta accanto è nascosta dal filo nero dell'antenna. Per la posizione
>   è molto probabilmente l'uscita "Lights AC220V" del manuale, cioè **a 230 V**.
>
> Le foto **non bastano** a dire che tensione c'è: lo dirà solo la misura
> dell'elettricista (E1).

**C5a — Morsetto verde a 3 posizioni ("ope / sto / clo")**

- **Foto:**
  - di lato, **da sinistra** e un po' dall'alto, in modo che si leggano le
    scritte complete accanto a ciascun polo (oggi si vede solo l'inizio);
  - una foto dritta del morsetto.
- **Da annotare:**
  - la scritta completa accanto a ciascuno dei tre poli, dall'alto in
    basso;
  - se ci sono fili collegati (nella foto 7 non se ne vedono).
- **Perché:** confermare che sono davvero gli ingressi "open", "stop" e
  "close" (H4a). Il morsetto **non ha un polo "comune"**: quale sia il
  comune lo dirà la misura dell'elettricista (H5).

**C5b — Morsetto verde a 2 posizioni vicino al fusibile ⚡ (non toccare)**

- **Foto:** da un'angolazione diversa, in modo che la scritta accanto non
  sia coperta dal filo nero dell'antenna. **Non spostare il filo**:
  cambia tu posizione.
- **Da annotare:**
  - la scritta;
  - se ci sono fili collegati e dove vanno (per esempio a un lampeggiante).
- **Perché:** confermare che è l'uscita del lampeggiante a 230 V (H4b). A
  questo morsetto **non va mai collegato nulla** dell'automazione del tetto.

## C6 — Scritte stampate vicino al morsetto verde

- **Foto:** ravvicinata delle scritte bianche intorno al morsetto verde.
  Se la scheda si vede anche da sotto, senza smontarla, fotografa anche il
  lato delle saldature.
- **Da annotare:** se compare una di queste parole: LAMP, LIGHT, AC220V,
  OPEN, STOP, CLOSE, COM, GND.
- **Perché:** aiuta a capire se quel morsetto è a 230 V o a bassa tensione
  (H4, H8).

## C7 — Fotocellule

- **Da annotare:** ci sono fotocellule montate (due "occhietti" uno di
  fronte all'altro)? Sono collegate alla scheda?
- **Foto:** delle fotocellule, se ci sono.
- **Perché:** capire se l'ingresso "Photo" è usato (H7).

## C8 — I DIP rossi

- **Foto:** ravvicinatissima, con una luce di lato, in modo che si leggano
  la scritta **ON** stampata sul blocchetto rosso e i numeri 1-2-3-4.
- **Da annotare:** per ogni levetta 1, 2, 3, 4: è dalla parte "ON" o
  dall'altra?
- **Perché:** serve a sapere com'è impostata oggi la **richiusura
  automatica** (DIP 2 e 3), che deve essere spenta. Riguarda anche l'avvio
  lento e il rilevamento ostacoli (H11, H12). **Non spostare le levette.**

## C9 — Il potenziometro "Closing Obstruction"

- **Foto:** dall'alto, in modo che si veda dove punta la tacca.
- **Da annotare:** la posizione, come se fosse un orologio (es. "punta
  alle 2").
- **Perché:** il manuale dice che regola la richiusura automatica, la
  scheda dice "ostacolo". Prima di cambiarlo dobbiamo sapere com'è adesso
  (H13).

## C10 — Prova di continuità (solo dopo C1, con l'elettricista)

- **Cosa fare:**
  1. Accendi il tester in modalità **continuità**: è il simbolo che fa
     "bip" quando tocchi i due puntali tra loro.
  2. Appoggia un puntale sul morsetto **GND** della morsettiera a vite.
  3. Con l'altro puntale tocca, uno alla volta, i tre poli del morsetto
     verde.
- **Da annotare:** per ciascun polo, se fa "bip" sì o no.
- **Perché:** se uno dei poli verdi fa "bip" con GND, il morsetto verde è
  quasi certamente a bassa tensione e quel polo potrebbe essere il comune
  (H4, H5). Se nessuno fa "bip", non si può concludere nulla: decideranno
  le misure dell'elettricista (E1).

## C11 — Connettore "Limit", selettori e morsettiera del motore ⚡ (solo guardare)

- **Foto:**
  - il connettore "Limit" con le scritte (CL, GND, OP?);
  - i due selettori ("one/two Limit" e "Motor Rotation"), in modo che si
    veda da che parte sono;
  - la morsettiera del motore con i fili, **senza toccarla**.
- **Da annotare:** da che parte è ciascun selettore; i colori dei fili del
  motore.
- **Perché:** sono impostazioni che **non vanno cambiate** e che servono a
  capire come la scheda ferma il motore a fine corsa (H14, H20).

## C12 — Il fusibile e la scritta ABM241

- **Foto:**
  - il fusibile (10 A?);
  - la scritta ABM241 con tutte le altre sigle o date stampate vicino.
- **Perché:** identificare esattamente la versione della scheda.

## C13 — Il manuale, tutte le pagine

- **Foto:** **ogni pagina**, anche quelle in cinese e le pagine con i
  disegni. Soprattutto:
  - la pagina "Power supply test" e quelle successive;
  - le pagine sui morsetti, sulle fotocellule e sulla richiusura automatica;
  - il seguito della frase "The LED flicker once…".
- **Perché:** il manuale e la scheda non corrispondono del tutto. Più
  pagine ho, meno ipotesi restano (H11-H13, H19).

## C14 — Telecomandi e chiave di sblocco

- **Da annotare:**
  - quanti telecomandi hai, e se uno può essere dedicato all'automazione
    (strada B);
  - se hai la **chiave di sblocco manuale** del motore, e dove si
    inserisce.
- **Foto:** il punto del motore dove si inserisce la chiave di sblocco.
- **Perché:**
  - il telecomando di scorta serve per la strada B (H10, H16);
  - la chiave serve per il collaudo con il motore che gira "a vuoto" (H21).

## C15 — Misure con il metro

- **Da annotare:**
  - la **lunghezza della corsa** del tetto, da tutto chiuso a tutto aperto
    (in metri);
  - con il telescopio parcheggiato, lo **spazio libero** tra il punto più
    alto del telescopio e la parte più bassa del tetto che gli passa sopra
    (in centimetri);
  - la distanza tra la scheda del cancello e il punto dove metterai la
    scatola dell'Arduino (per la lunghezza dei cavi).
- **Perché:**
  - la corsa serve per calcolare il tempo massimo del movimento (H17);
  - lo spazio libero serve per la sicurezza del telescopio.

---

## E — Misure CON la corrente: solo l'elettricista ⚡

Queste misure non vanno fatte da te. Le scrivo perché tu possa chiederle
all'elettricista.

- **E1. Tensioni.** Tester in tensione continua, puntale nero sul GND della
  morsettiera a vite. Misurare la tensione su ogni morsetto (+15V, Photo,
  OPN, test) e su **ogni polo del morsetto verde**. Poi ripetere in
  **tensione alternata** sul morsetto verde.
  - Se c'è tensione alternata intorno a 230 V, è il lampeggiante (H3, H4,
    H5, H8).
- **E2. Richiusura automatica.** Con il motore **sbloccato** (chiave di
  sblocco), dare un comando di apertura col telecomando e aspettare 60
  secondi. Il motore **non** deve ripartire da solo in chiusura (H12, H13).
- **E3. Terra e collegamenti.** Verificare la messa a terra della carcassa
  del motore e la corretta posizione di fase e neutro, come chiede il
  manuale (H20).

---

## Cosa mandarmi alla fine

1. Le foto, chiamate C2, C3, … come qui sopra.
2. Le annotazioni, anche come foto di questo foglio compilato a penna.
3. I valori misurati dall'elettricista (E1, E2, E3).

Con questi dati aggiornerò **solo**:

- il file `arduino/tetto/configurazione_scheda.h`;
- le istruzioni di cablaggio in `docs/03-collegamenti.md`;
- lo stato delle ipotesi in `docs/09-ipotesi-da-verificare.md`.

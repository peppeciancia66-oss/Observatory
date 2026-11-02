# 4. La scheda del cancello (ABM241): cosa sappiamo e cosa no

> ⚠️ **Niente di questa pagina è ancora verificato con le misure.** Le
> letture vengono dalle foto del 2 novembre 2026 e dal manuale. Le foto
> possono confermare **solo ciò che è scritto o visibile**, non le tensioni
> né il funzionamento.
>
> Non collegare nulla alla scheda finché non sono fatti i controlli di
> [10-controlli-senza-corrente.md](10-controlli-senza-corrente.md) e
> verificate le ipotesi di [09-ipotesi-da-verificare.md](09-ipotesi-da-verificare.md),
> dove trovi lo stato di ciascuna: 📷 "confermato dalla foto" oppure
> 📏 "da misurare".
>
> Tutto ciò che riguarda i **230 V** lo fa un **elettricista** ⚡.

## 4.1 Le foto ricevute

Le foto ricevute sono **sette**: le prime cinque con il primo messaggio,
le ultime due (scheda intera e morsetti) con il secondo.

| # | Contenuto | Leggibilità |
|---|---|---|
| 1 | Manuale: telecomando, abbinamento, "Use a function" | Buona. Pagina tagliata in basso ("The LED flicker once…") e a destra |
| 2 | Manuale: schema della scheda con le didascalie | Buona per le didascalie |
| 3 | Manuale: "Motor wiring diagram" (P6) | Buona |
| 4 | Scheda vera: parte in alto a sinistra (tasto di apprendimento, potenziometro, DIP) | Scura e sfocata |
| 5 | Targhetta del motore | Buona |
| 6 | Scheda vera: fila di morsetti "+15V out … test" e connettore "Limt" | Buona |
| 7 | Scheda vera: intera | Buona nell'insieme; dettagli piccoli (DIP, scritte vicino ai morsetti verdi) non leggibili con sicurezza |

## 4.2 Cosa ho letto

**Targhetta del motore** (foto 5), lettura sicura:

- modello **MD370**;
- potenza **370 W**;
- alimentazione **AC 240 V~ 50 Hz**;
- corrente di spunto **3 A**;
- velocità del motore 1400 giri/min;
- velocità di traslazione **12 m/min** (0,2 m/s).

**Telecomando** (foto 1, manuale), lettura sicura:

- 4 tasti: **Open** (▲, in alto a sinistra), **close** (▼, in alto a
  destra), **Stop** (■, in basso a destra), **No use** (●, in basso a
  sinistra);
- 433 MHz, chip EV1527, pila 27A da 12 V;
- tenendo premuto un tasto per più di 4 secondi, il telecomando smette di
  trasmettere.

**Scheda vera** (foto 6 e 7):

| Dove | Cosa si legge | Note |
|---|---|---|
| Fila di 5 morsetti a vite, in basso | **+15V out**, **GND**, **Photo**, **OPN**, **test** (da sinistra) | Ben leggibile. Non si vedono fili né ponticelli |
| Accanto alla fila | **ABM241**, "Date 2024-1…" (data tagliata) | |
| Connettore bianco a destra della fila | **CL**, **GND**, **OP**, e sotto "**Limt**" | Ha fili collegati; non si vede dove vanno |
| Connettore successivo | Solo "**M**…" (tagliata) | Fili rossi e nero |
| **Morsetto verde a 3 posizioni**, in basso a sinistra | Solo l'inizio delle scritte: "**ope**…", "**sto**…", "**clo**…" | Il resto è coperto dal morsetto. Non ha un polo "comune". Non si vedono fili collegati |
| **Morsetto verde a 2 posizioni**, a destra vicino al fusibile | Scritta **illeggibile**, coperta dal filo nero dell'antenna | Si trova nella zona della corrente: accanto a "**FUSE**", ai due relè grandi, al trasformatore e ai morsetti con fili blu e marrone (dove si legge solo "…**0V**") |
| Due relè grandi | **HKE JQX15F-90S, 30A 250VAC / 28VDC, COIL 12VDC** | Comandano il motore: zona a 230 V ⚡ |
| DIP rosso | "**ON**" in alto sul blocchetto e "**DIP**"; numeri **1 2 3 4**. Sopra: "**soft-start**" (1), "**Self closing**" (2-3), "**Obstruction**" (4) | Posizione delle levette **non leggibile con sicurezza** |
| Tabellina vicino al DIP | "…OFF 5", "OFF ON 10", "ON ON 20" | Tempi della richiusura automatica. La riga "OFF OFF" **non si vede** |
| Potenziometro | "**Closing Obstruction**" con frecce "+" | |
| Selettori | "**two / one Limt**" e "**Motor Ro…**" (Motor Rotation) | La posizione dei selettori non è chiara |
| Pulsanti | **K1** (apprendimento: "1S Enter learning key", "8S Let go of the Key Delete the code") e **K2** "**test**" | |

**Schema della scheda nel manuale** (foto 2), a confronto con la scheda vera:

| Didascalia nel manuale | Sulla scheda vera |
|---|---|
| "1.2 Output 15 VDC, 30 mA", "2.3 infrared", "2.4 Open", "2.5 cycle" | Corrisponde alla fila +15V out, GND, Photo, OPN, test, con "test" al posto di "cycle". Il morsetto 2 è "GND" |
| "Lights AC220V", con freccia verso un morsetto verde | Probabilmente il **morsetto verde a 2 posizioni** vicino al fusibile (lampeggiante a 230 V). È da misurare |
| "1.ON Soft start" | DIP 1 "soft-start" ✔ |
| "Autoclosing 2ON3OFF/5S, 2OFF3ON/10S, 2ON3ON/20S" | DIP 2-3 "Self closing" e la tabellina ✔ |
| "4.ON/Resista" | DIP 4 "Obstruction" ✔ |
| "Single limit and double limit" | Selettore "two / one Limt" ✔ |
| "turns forward and backward" | Selettore "Motor Rotation" ✔ |
| "fuse 10A/250" | Componente "FUSE" ✔ |

**Schema del motore nel manuale** (foto 3): morsettiera con terra ("Ground
line"), comune ("Public line"), due fili di direzione ("Direction line") e
tre fili del finecorsa ("Limit line"). I fili del motore sono **a 230 V** ⚡.

## 4.3 Cosa è chiarito e cosa no

**Chiarito dalle foto:**

1. **I morsetti verdi sono due.** Il conflitto con il manuale sembra
   spiegato: quello a 3 posizioni porta le scritte "ope / sto / clo",
   mentre la didascalia "Lights AC220V" si riferisce probabilmente a quello
   a 2 posizioni vicino al fusibile.
2. **Le scritte della fila a vite** sono +15V out, GND, Photo, OPN, test,
   nello stesso ordine dello schema del manuale.
3. **Le funzioni dei DIP** sono scritte sulla scheda: la richiusura
   automatica si regola con i DIP 2-3 ("Self closing"), non con il
   potenziometro. Il potenziometro porta la scritta "Closing Obstruction".

**Ancora da chiarire (con foto ravvicinate, misure o collaudo):**

1. **Le scritte complete** del morsetto a 3 posizioni: oggi si legge solo
   l'inizio ("ope", "sto", "clo") (H4a, controllo C5a).
2. **Le tensioni** dei due morsetti verdi e della fila a vite. Il morsetto
   a 3 posizioni si trova nella zona a bassa tensione, ma una foto non lo
   prova. Il morsetto a 2 posizioni, fino alla misura, va trattato **come a
   230 V** (H3, H4a, H4b, misura E1 dell'elettricista).
3. **Il comune** del morsetto a 3 posizioni: probabilmente il GND della
   fila a vite, ma va misurato (H2, H5).
4. **La posizione dei DIP** e se con 2 e 3 su OFF la richiusura automatica
   è davvero spenta. La riga "OFF OFF" non si vede nella tabellina (H11,
   H12, controllo C8, misura E2).
5. **Il potenziometro.** Il manuale dice che regola la richiusura
   automatica; la scheda dice "Closing Obstruction" e la richiusura ha i
   suoi DIP. La versione del manuale è quindi meno probabile, ma non esclusa
   (H13).
6. **L'ingresso "test".** Corrisponde probabilmente al "cycle" del manuale
   (passo-passo) e al pulsante K2 "test". **Non lo useremo per il tetto.**
7. **L'ingresso "Photo".** Oggi non ha fili né ponticelli: va verificato
   al collaudo che la scheda chiuda lo stesso (H7).
8. **Dopo un blackout** la scheda potrebbe non sapere dov'è il tetto (H19,
   collaudo C.6).
9. **I selettori** "one/two Limt" e "Motor Rotation": la posizione non è
   chiara, e comunque **non vanno toccati** (H14).
10. **Documentazione.** Della ABM241 non ho trovato documentazione
    pubblica, solo di schede simili. Nessuna informazione "presa da
    internet" viene usata come certa.

## 4.4 Le due strade possibili (decideremo con le misure)

Lo sketch dell'Arduino è **lo stesso** nei due casi: tre relè, cioè tre
"pulsanti" APRI, CHIUDI e STOP. Cambiano solo i fili e qualche valore in
`arduino/tetto/configurazione_scheda.h`.

**Strada A — ingressi della scheda.** È la più probabile dopo le foto, ma
vale **solo se** le misure confermano che il morsetto verde a 3 posizioni è
"open / stop / close" a bassa tensione:

- i tre relè vanno ai tre ingressi;
- il comune va al morsetto indicato dalla misura.

**Strada B — telecomando di scorta in parallelo.** Si usa se le misure
dicono che gli ingressi non sono utilizzabili:

- si abbina alla scheda un **secondo telecomando** uguale;
- si collegano i relè ai suoi tasti Open, close e Stop.

Il telecomando funziona a pila (12 V) ed è isolato dalla scheda. Lo
svantaggio è che passa via radio: se un comando non arriva, però, l'Arduino
se ne accorge ("il tetto non è partito") e blocca tutto.

**In nessun caso:**

- si usa l'ingresso "test" (passo-passo);
- si collega qualcosa al morsetto verde a 2 posizioni;
- si collega l'Arduino direttamente ai morsetti della scheda.

## 4.5 Quale relè

- **Strada A.** Gli ingressi sono probabilmente alimentati dai 15 V della
  scheda, con correnti di pochi milliampere. Il modulo a 4 relè della lista
  va bene; per correnti così piccole sono ancora più affidabili i relè con
  **contatti dorati** (lista della spesa, voce A3).
- **Strada B.** I tasti del telecomando lavorano con correnti piccolissime:
  anche qui vanno bene, e sono anzi preferibili, i contatti dorati.
- **In entrambi i casi:** se le misure dessero più di 30 V, o tensione
  alternata, ci si ferma e decide l'elettricista.

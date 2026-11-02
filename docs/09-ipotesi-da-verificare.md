# 9. Ipotesi da verificare

Questo file elenca **tutto ciò che ho dato per probabile ma non ho
verificato**. Per ogni ipotesi trovi:

- **Base:** da dove viene;
- **Dove pesa:** quale file o parametro ne dipende;
- **Come si verifica:** il numero del controllo in
  [10-controlli-senza-corrente.md](10-controlli-senza-corrente.md) (C…), o
  del collaudo in [07-collaudo.md](07-collaudo.md);
- **Se è sbagliata:** che cosa cambia;
- **Stato** dopo le foto 6 e 7.

Quando i dati arriveranno, aggiornerò **solo**
`arduino/tetto/configurazione_scheda.h` e le istruzioni di cablaggio in
[03-collegamenti.md](03-collegamenti.md), e segnerò qui il nuovo stato.

**Legenda dello stato:**

- 📷 **confermato dalla foto:** si legge chiaramente nella foto 6 o 7.
  Riguarda solo **ciò che è scritto o visibile** sulla scheda, **non** la
  tensione e **non** il funzionamento;
- 📏 **da misurare:** serve una misura (dell'elettricista ⚡) o una prova al
  collaudo;
- ⚡ coinvolge i 230 V: se ne occupa **solo l'elettricista**.

Quando un'ipotesi ha una parte confermata e una no, lo stato è 📏 e la nota
dice quale parte la foto conferma.

## Lettura delle foto 6 e 7 (2 novembre 2026)

| Dove | Cosa si legge davvero | Note |
|---|---|---|
| Fila di 5 morsetti a vite, in basso | **+15V out**, **GND**, **Photo**, **OPN**, **test** (da sinistra a destra) | Ben leggibile (foto 6). Non si vedono fili collegati |
| Accanto alla fila | **ABM241**, "Date 2024-1…" (data tagliata) | |
| Connettore bianco a destra della fila | **CL**, **GND**, **OP** e sotto "**Limt**" | Ci sono dei fili collegati (blu e scuri), ma da qui non si vede dove vanno |
| Connettore successivo | Solo una "**M**…" (tagliata) | Fili rossi e nero |
| Morsetto verde a 3 posizioni, a sinistra | Solo l'inizio: "**ope**…", "**sto**…", "**clo**…" | Il resto è coperto dal morsetto. Non si vedono fili collegati |
| Morsetto verde a 2 posizioni, a destra | Scritta **illeggibile** (coperta dal filo dell'antenna). Più in basso, vicino ai morsetti con i fili blu e marrone, si vede solo "…**0V**" | Si trova nella zona della corrente: accanto a "**FUSE**", ai due relè grandi e al trasformatore |
| Due relè grandi | **HKE JQX15F-90S, 30A 250VAC / 28VDC, COIL 12VDC** | Comandano il motore: zona a 230 V ⚡ |
| DIP rosso | Sul blocchetto: "**ON**" in alto a sinistra e "**DIP**"; numeri **1 2 3 4**. Sopra: "**soft-start**" (1), "**Self closing**" (2-3), "**Obstruction**" (4) | La posizione delle levette nella foto 7 è troppo piccola per essere sicura |
| Tabellina vicino al DIP | Si leggono solo "…OFF 5", "OFF ON 10", "ON ON 20" | Tempi di richiusura automatica. La riga "OFF OFF" **non si vede** |
| Potenziometro | "**Closing Obstruction**", con frecce "+" | |
| Selettori | "**two** / **one** **Limt**" e "**Motor Ro…**" (Motor Rotation) | La posizione dei selettori non è chiara |
| Pulsanti | **K1** (apprendimento: "1S Enter learning key", "8S Let go of the Key Delete the code") e **K2** "**test**" | |

## Scheda del cancello: ingressi di comando

| ID | Ipotesi | Base | Dove pesa | Come si verifica | Se è sbagliata | Stato |
|---|---|---|---|---|---|---|
| H1 | La fila a 5 morsetti è, da sinistra: +15 V uscita, GND/comune, Photo (fotocellula), OPN (apri), test/cycle | Manuale + tue letture | `docs/03` | C3, E1 | Si rifà lo schema dei collegamenti | 📷 **confermato dalla foto** per le **scritte e l'ordine**. 📏 **da misurare:** tensione e funzione di ciascun morsetto (E1). "test" corrisponde probabilmente al "cycle" del manuale e al pulsante K2 "test", ma non è verificato |
| H2 | Il **comune** degli ingressi è il morsetto GND | Manuale (didascalie che iniziano con "2.") | `docs/03` (dove va il COM dei relè) | C10, E1 | Il COM dei relè va su un altro morsetto | 📏 **da misurare.** La foto conferma che esiste un morsetto "GND" nella fila. Che sia il comune degli ingressi, e in particolare del morsetto verde a 3 posizioni, la foto non lo dice |
| H3 | Gli ingressi sono a **bassa tensione continua** (circa 15 V, pochi mA) | "+15V out" sulla stessa fila | Scelta del modulo relè (A3) | E1 | Se > 30 V o alternata: decide l'elettricista | 📏 **da misurare** (E1) |
| H4a | Il **morsetto verde a 3 posizioni** (a sinistra) è l'ingresso **open / stop / close** | Scritte "ope…", "sto…", "clo…" | Strada A o B | C5a, E1 | Strada B (telecomando di scorta) | 📏 **da misurare.** La foto conferma solo l'**inizio** delle scritte ("ope", "sto", "clo"). La posizione, accanto alla fila a bassa tensione e lontano da relè, fusibile e trasformatore, fa pensare alla bassa tensione, ma **non è una prova** |
| H4b ⚡ | Il **morsetto verde a 2 posizioni** (vicino al fusibile) è l'uscita **"Lights AC220V"** del manuale, cioè il lampeggiante **a 230 V** | Manuale + posizione nella zona della corrente | Sicurezza | C5b, E1 | — (non va comunque mai usato per il tetto) | 📏 **da misurare.** La scritta è coperta dal filo dell'antenna. Fino alla misura va trattato **come a 230 V** |
| H5 | Il comune del morsetto verde a 3 posizioni è il GND della fila a 5 morsetti | Il morsetto verde non ha un polo "comune" proprio (confermato dalla foto: ha solo 3 poli) | `docs/03` | C10, E1 | Si cerca il comune giusto | 📏 **da misurare** |
| H6 | "OPN" è **solo apri**, non passo-passo | Manuale "2.4 Open" | Strada A | Collaudo B | Non si usa | 📏 **da misurare.** La foto conferma la scritta "OPN" |
| H7 | L'ingresso "Photo" non serve al tetto e la scheda chiude anche senza fotocellula | Manuale "2.3 infrared" | Funzionamento della chiusura | C4, C7, collaudo B | Se la scheda rifiuta la chiusura, serve un ponticello o un'impostazione | 📏 **da misurare.** La foto conferma che **oggi non c'è nessun filo né ponticello** sul morsetto Photo |
| H8 | L'ingresso STOP si comanda **chiudendo** un contatto (NA) | Comune nelle schede economiche | `docs/03` (NO o NC del relè STOP) | E1, collaudo B | Si usa il contatto NC del relè 3 | 📏 **da misurare** |
| H9 | Premere STOP a **motore fermo** non fa nulla | Comportamento abituale | `FERMA_AL_FINECORSA`, `STOP_ANCHE_DA_FERMO` | Collaudo B | Si mettono a 0 | 📏 **da misurare** (collaudo) |
| H10 | Il tasto **Stop** del telecomando ferma il motore in qualsiasi momento | Manuale del telecomando | Strada B | Collaudo B | Strada B non utilizzabile così | 📏 **da misurare** (collaudo) |
| H15 | Un impulso di **0,5 s** basta perché la scheda riconosca il comando | Comportamento abituale + manuale (il telecomando smette dopo 4 s) | `IMPULSO_MS` | Collaudo B | Si allunga (es. 1000 ms, mai oltre 3000) | 📏 **da misurare** (collaudo) |
| H16 | Un telecomando di scorta può essere abbinato alla scheda | Manuale "Learn key: 40 PCS" + pulsante K1 sulla scheda | Strada B | Collaudo B | Altra soluzione | 📏 **da misurare.** La foto conferma il pulsante K1 con le istruzioni di apprendimento |

## Scheda del cancello: impostazioni e comportamento

| ID | Ipotesi | Base | Dove pesa | Come si verifica | Se è sbagliata | Stato |
|---|---|---|---|---|---|---|
| H11 | DIP 1 = avvio lento, 2-3 = richiusura automatica, 4 = ostacoli. Lato ON in alto | Manuale + scritte sulla scheda | `TEMPO_MAX_DISTACCO_S`, sicurezza | C8 | Si rivede la tabella | 📏 **da misurare.** 📷 **Confermate dalla foto** le **funzioni** ("soft-start", "Self closing", "Obstruction") e la scritta "ON" in alto sul blocchetto. **Non confermata** la posizione attuale delle levette. Dalla foto 4 sembravano 1 e 4 da una parte, 2 e 3 dall'altra; con "ON" in alto sarebbero **1 e 4 su ON, 2 e 3 su OFF**, ma le foto sono troppo piccole o scure per esserne sicuri: serve la foto ravvicinata C8 |
| H12 | Con DIP 2 = OFF e 3 = OFF la **richiusura automatica è disattivata** | La tabellina elenca solo combinazioni con un tempo | **Sicurezza** | C8, E2, collaudo B.3 | Va trovato come disattivarla; fino ad allora il tetto **non si usa in automatico** | 📏 **da misurare.** La foto conferma che la richiusura automatica si regola con i DIP 2-3 ("Self closing"); la riga "OFF OFF" nella tabellina non si vede |
| H13 | Il potenziometro regola la **sensibilità agli ostacoli**, non la richiusura automatica | Scritta sulla scheda | Sicurezza | C9, collaudo C.4 | Se regola la richiusura, va al minimo come dice il manuale | 📏 **da misurare.** La foto conferma la scritta "Closing Obstruction" e che la richiusura ha i suoi DIP ("Self closing"). Questo rende meno probabile la versione del manuale, ma non la esclude |
| H14 | La scheda si ferma da sola con i **finecorsa interni del motore** (connettore "Limit" CL/GND/OP) e il selettore one/two Limit è impostato bene | Manuale + scritte | `FERMA_AL_FINECORSA`, fermi meccanici | C11, collaudo C.1 | Vanno regolati i finecorsa interni | 📏 **da misurare.** 📷 **Confermate dalla foto** le scritte "CL GND OP" e "Limt" e la presenza di fili nel connettore. **Non visibili** la posizione del selettore e dove vanno i fili |
| H19 | Dopo un **blackout** la scheda non sa dove si trova il tetto | Manuale, "Use a function" punti 1-2 | Collaudo, rischio | Collaudo C.6 | — | 📏 **da misurare** (collaudo) |
| H20 ⚡ | Il collegamento di motore, alimentazione e terra è corretto | Il manuale lo richiede | Sicurezza | C2, C11, E3 | Lo sistema l'elettricista | 📏 **da misurare** (elettricista). La foto mostra fili blu e marrone sui morsetti a destra: è la parte a 230 V |

## Tetto e motore

| ID | Ipotesi | Base | Dove pesa | Come si verifica | Se è sbagliata | Stato |
|---|---|---|---|---|---|---|
| H17 | La corsa completa dura **al massimo circa 30 s** (cremagliera da 6 m a 12 m/min) | Targhetta + tua descrizione | `TEMPO_MAX_MOVIMENTO_S = 45`, `TEMPO_MAX_DISTACCO_S = 8` | C15, collaudo C.3 | Si aggiornano i due tempi | 📏 **da misurare** |
| H21 | Con il motore sbloccato il motore gira senza muovere il tetto | Funzione standard | Collaudo fase B | C14 | La fase B si fa con il motore scollegato dall'elettricista | 📏 **da misurare** |

## Componenti miei (non della scheda)

| ID | Ipotesi | Base | Dove pesa | Come si verifica | Se è sbagliata | Stato |
|---|---|---|---|---|---|---|
| H18 | Il modulo relè comprato è "attivo basso" | La maggior parte dei moduli | `RELE_ATTIVO_BASSO = 1` | Collaudo A.2 | Si mette 0 | 📏 **da misurare** (collaudo) |
| H22 | L'Hydreon RG-11 ha un contatto "chiuso quando è asciutto" | Manuale Hydreon (da controllare) | `docs/03` (3.4) | Collaudo A.5 | Si cambia il collegamento del relè di appoggio | 📏 **da misurare** (collaudo) |
| H23 | Senza sensore di movimento, un movimento non comandato si riconosce **solo** quando il tetto lascia un finecorsa | Limite del progetto, verificato con le prove al simulatore | `USA_SENSORE_MOVIMENTO` | — | Consiglio di montare il sensore di movimento (A13) | Non riguarda la scheda né le foto: è un **limite noto** del progetto |

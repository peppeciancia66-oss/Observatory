# 9. Ipotesi da verificare

Questo file elenca **tutto ciò che ho dato per probabile ma non ho
verificato**. Per ogni ipotesi trovi:

- **Base:** da dove viene;
- **Dove pesa:** quale file o parametro ne dipende;
- **Come si verifica:** il numero del controllo in
  [10-controlli-senza-corrente.md](10-controlli-senza-corrente.md) (C…), o
  del collaudo in [07-collaudo.md](07-collaudo.md);
- **Se è sbagliata:** che cosa cambia.

Quando i dati arriveranno, aggiornerò **solo**
`arduino/tetto/configurazione_scheda.h` e le istruzioni di cablaggio in
[03-collegamenti.md](03-collegamenti.md), e segnerò qui ogni ipotesi come ✅
verificata o ❌ smentita.

Legenda: ⏳ da verificare · ✅ verificata · ❌ smentita · ⚡ coinvolge i 230 V (elettricista)

## Scheda del cancello: ingressi di comando

| ID | Ipotesi | Base | Dove pesa | Come si verifica | Se è sbagliata | Stato |
|---|---|---|---|---|---|---|
| H1 | La morsettiera bassa a 5 posizioni è: 1 = +15 V uscita, 2 = GND/comune, 3 = Photo (fotocellula), 4 = OPN (apri), 5 = cycle/"test" | Manuale ("1.2 Output 15VDC", "2.3 infrared", "2.4 Open", "2.5 cycle") + tue letture | `docs/03`, scelta tra strada A e B | C3, C4 | Si rifà lo schema dei collegamenti | ⏳ |
| H2 | Il **comune** degli ingressi di comando è il morsetto 2 (GND) | Manuale: tutte le didascalie iniziano con "2." | `docs/03` (dove va il COM dei relè) | C4, C10 | Il COM dei relè va su un altro morsetto | ⏳ |
| H3 | Gli ingressi sono a **bassa tensione continua** (circa 15 V, pochi mA): basta un contatto pulito per dare il comando | Uscita "15 VDC 30 mA" sulla stessa morsettiera | Scelta del modulo relè (A3) | E1 (misura con l'elettricista) | Se > 30 V o alternata: niente modulo, decide l'elettricista | ⏳ |
| H4 ⚡ | Il **morsetto verde a 3 posizioni** potrebbe essere l'uscita **"Lights AC220V"** (lampeggiante a 230 V) e **non** "open / stop / close" | Didascalia del manuale con freccia verso un morsetto verde. **In conflitto** con la tua lettura | Strada A o B; **sicurezza** | C5, C6, E1 | Se è davvero "open/stop/close" a bassa tensione → strada A | ⏳ |
| H5 | Se il morsetto verde è "open / stop / close", il suo comune è il GND della morsettiera a 5 posizioni | Tre morsetti senza un comune proprio | `docs/03` | C6, C10, E1 | Si cerca il comune giusto | ⏳ |
| H6 | "OPN" è **solo apri**, non passo-passo | Manuale "2.4 Open" | Strada A (relè APRI) | C4, collaudo B | Se è passo-passo, non si usa: strada B | ⏳ |
| H7 | L'ingresso "Photo" è per fotocellule e **non serve** al tetto. Oggi è ponticellato (o non necessario) | Manuale "2.3 infrared" | Funzionamento della chiusura | C4, C7 | Se manca il ponticello, la scheda può rifiutare la chiusura | ⏳ |
| H8 | L'ingresso STOP (se esiste) si comanda **chiudendo** un contatto (NA) | Comune nelle schede economiche | `docs/03` (NO o NC del relè STOP) | C6, E1 | Se lo STOP è NC: si usa il contatto NC del relè 3 | ⏳ |
| H9 | Premere STOP a **motore fermo** non fa nulla | Comportamento abituale | `FERMA_AL_FINECORSA`, `STOP_ANCHE_DA_FERMO` | Collaudo B | Si mettono a 0 | ⏳ |
| H10 | Il tasto **Stop** del telecomando ferma il motore in qualsiasi momento | Manuale del telecomando | Strada B | Collaudo B (motore sbloccato) | Strada B non utilizzabile così | ⏳ |
| H15 | Un impulso di **0,5 s** basta perché la scheda riconosca il comando, sia via ingressi sia via telecomando (che smette di trasmettere dopo 4 s) | Comportamento abituale + manuale (4 s) | `IMPULSO_MS` | Collaudo B | Si allunga (es. 1000 ms, mai oltre 3000) | ⏳ |
| H16 | Il telecomando di scorta può essere abbinato alla scheda (fino a 40) | Manuale: "Learn key: 40 PCS" | Strada B | Collaudo B | Si cerca un'altra soluzione | ⏳ |

## Scheda del cancello: impostazioni e comportamento

| ID | Ipotesi | Base | Dove pesa | Come si verifica | Se è sbagliata | Stato |
|---|---|---|---|---|---|---|
| H11 | DIP 1 = avvio lento, 2-3 = richiusura automatica, 4 = rilevamento ostacoli. Nella foto 4 sembrano 1 e 4 da una parte, 2 e 3 dall'altra; **non so quale lato sia ON** | Manuale + foto 4 (scura) | `TEMPO_MAX_DISTACCO_S`, sicurezza | C8 | Si rivede la tabella | ⏳ |
| H12 | Con DIP 2 = OFF e 3 = OFF la **richiusura automatica è disattivata** | Il manuale elenca solo le combinazioni con un tempo | **Sicurezza** | C8 + collaudo B ("aspetta 60 s dopo un'apertura") | Va trovato come disattivarla (potenziometro? H13). Fino ad allora il tetto **non si usa in automatico** | ⏳ |
| H13 | Il potenziometro regola la **sensibilità agli ostacoli** ("Closing Obstruction") e non la richiusura automatica | Scritta sulla scheda, in contrasto con il manuale (punto 4) | Sicurezza | C9 + collaudo B/C | Se regola la richiusura, va messo al minimo (antiorario) come dice il manuale | ⏳ |
| H14 | La scheda si ferma da sola con i **finecorsa interni del motore** (connettore "Limit" CL/GND/OP) e il selettore "one/two Limit" è già impostato correttamente | Manuale + tue letture | `FERMA_AL_FINECORSA`, fermi meccanici | C11, collaudo C.1 | Bisogna regolare i finecorsa interni prima di tutto | ⏳ |
| H19 | Dopo un **blackout** la scheda non sa dove si trova il tetto: il primo comando potrebbe muoverlo in un verso diverso da quello chiesto | Manuale, punti 1 e 2 di "Use a function" | Collaudo, rischio | Collaudo C (con l'elettricista che stacca e riattacca la corrente) | — (se è vero, lo sketch se ne accorge solo al finecorsa opposto: dopo un blackout la prima manovra va sorvegliata) | ⏳ |
| H20 ⚡ | Il collegamento attuale di motore, alimentazione e terra è corretto (fase e neutro non invertiti, carcassa a terra) | Il manuale lo richiede | Sicurezza | C2, C11 + elettricista | Lo sistema l'elettricista | ⏳ |

## Tetto e motore

| ID | Ipotesi | Base | Dove pesa | Come si verifica | Se è sbagliata | Stato |
|---|---|---|---|---|---|---|
| H17 | La corsa completa dura **al massimo circa 30 s** (cremagliera da 6 m a 12 m/min) | Targhetta + tua descrizione | `TEMPO_MAX_MOVIMENTO_S = 45`, `TEMPO_MAX_DISTACCO_S = 8` | Collaudo C.3 (cronometro) | Si aggiornano i due tempi | ⏳ |
| H21 | Con il motore sbloccato (chiave di sblocco manuale) il motore gira senza muovere il tetto | Funzione standard dei motori per cancelli | Collaudo fase B | Manuale / prova | La fase B si fa con il motore scollegato dall'elettricista | ⏳ |

## Componenti miei (non della scheda)

| ID | Ipotesi | Base | Dove pesa | Come si verifica | Se è sbagliata | Stato |
|---|---|---|---|---|---|---|
| H18 | Il modulo relè comprato è "attivo basso" | La maggior parte dei moduli con optoisolatori | `RELE_ATTIVO_BASSO = 1` | Collaudo A.2 | Si mette 0 | ⏳ |
| H22 | Il contatto dell'Hydreon RG-11 "chiuso quando è asciutto" esiste ed è selezionabile | Manuale Hydreon (da controllare) | `docs/03` (3.4) | Collaudo A.5 | Si cambia il collegamento del relè di appoggio | ⏳ |
| H23 | Senza sensore di movimento, un movimento non comandato si riconosce **solo** quando il tetto lascia un finecorsa (non se parte da metà corsa) | Limite del progetto, verificato con le prove | `USA_SENSORE_MOVIMENTO` | — | Consiglio di montare il sensore di movimento (voce A13) | ✅ (è un limite noto) |

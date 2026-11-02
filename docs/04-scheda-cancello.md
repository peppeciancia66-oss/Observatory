# 4. La scheda del cancello (ABM241): cosa sappiamo e cosa no

> ⚠️ **Niente di questa pagina è ancora verificato sulla scheda vera.** Le
> letture vengono dalle foto del 2 novembre 2026. Non collegare nulla
> alla scheda finché non sono fatti i controlli di
> [10-controlli-senza-corrente.md](10-controlli-senza-corrente.md) e
> verificate le ipotesi di [09-ipotesi-da-verificare.md](09-ipotesi-da-verificare.md).

## 4.1 Le foto ricevute

Delle sette foto annunciate ne ho ricevute **cinque**:

| # | Contenuto | Leggibilità |
|---|---|---|
| 1 | Manuale: telecomando, abbinamento, "Use a function" | Buona. La pagina è tagliata in basso ("The LED flicker once…") e a destra |
| 2 | Manuale: schema della scheda con le didascalie | Buona per le didascalie; le scritte sulla scheda nella foto del manuale sono illeggibili |
| 3 | Manuale: "Motor wiring diagram" (P6) | Buona |
| 4 | Scheda vera: parte in alto a sinistra (tasto di apprendimento, potenziometro, DIP) | **Scura e sfocata** |
| 5 | Targhetta del motore | Buona |

**Non sono arrivate** la foto della scheda intera e quella dei morsetti, cioè
quella con la scritta ABM241. Quindi **non ho visto con i miei occhi** il
morsetto verde, i morsetti "+15V out / GND / Photo / OPN / test" e il
connettore "Limit". Per questi uso le tue letture e il manuale, e le tratto
come ipotesi.

## 4.2 Cosa ho letto (e quanto ne sono sicuro)

**Targhetta del motore** (foto 5), lettura sicura:

- Modello **MD370**
- Potenza **370 W**
- Alimentazione **AC 240 V~ 50 Hz**
- Corrente di spunto **3 A**
- Velocità del motore 1400 giri/min
- Velocità di traslazione **12 m/min** (cioè 0,2 m/s)

**Telecomando** (foto 1, manuale), lettura sicura:

- 4 tasti:
  - **Open** (▲, in alto a sinistra)
  - **close** (▼, in alto a destra)
  - **Stop** (■, in basso a destra)
  - **No use** (●, in basso a sinistra)
- 433 MHz, chip EV1527, pila 27A da 12 V.
- Tenendo premuto un tasto per più di 4 secondi, il telecomando smette di
  trasmettere.
- Risposta alla mia domanda di prima: il telecomando ha **tasti separati**
  per aprire, chiudere e fermare.

**Schema della scheda nel manuale** (foto 2), didascalie leggibili:

| Didascalia nel manuale | Mia interpretazione | Sicurezza |
|---|---|---|
| "1.2 Output 15 VDC, 30 mA" | Tra i morsetti 1 e 2 c'è un'uscita a 15 V per gli accessori | media |
| "2.3 infrared" | Tra 2 e 3: fotocellula ("Photo") | media |
| "2.4 Open" | Tra 2 e 4: comando apri ("OPN") | media |
| "2.5 cycle" | Tra 2 e 5: comando "ciclico" passo-passo (forse il tuo "test") | media |
| Morsetto 2 | **Comune** degli ingressi (il tuo "GND") | media |
| "Lights AC220V" con la freccia verso un **morsetto verde** | Uscita per il **lampeggiante a 230 V** | **⚠️ in conflitto con la tua lettura** |
| "1.ON open the Soft start / 1.OFF Off Soft start" | DIP 1 = avvio lento | buona |
| "Autoclosing 2ON3OFF/5S, 2OFF3ON/10S, 2ON3ON/20S" | DIP 2 e 3 = richiusura automatica dopo 5, 10 o 20 s | buona; **manca 2OFF3OFF** |
| "4.ON/Resista" | DIP 4 = rilevamento ostacoli ("resistenza") | media |
| "Resistance to adjust" | Potenziometro di regolazione della forza/ostacolo | media |
| "Single limit and double limit" | Selettore uno/due finecorsa (il tuo "one/two Limt") | buona |
| "turns forward and backward" | Selettore del verso del motore (il tuo "Motor Rotation") | buona |
| "limit", "motor" | Connettori del finecorsa e del motore | buona |
| "fuse 10A/250" | Fusibile da 10 A | buona |
| "The power of 220V/50HZ" | Alimentazione di rete ⚡ | buona |
| "Learn key: 40 PCS" | Fino a 40 telecomandi | buona |

**Scheda vera, foto 4** (scura):

- Scritte: "8S Let go of the Key Delete the code", "1S Enter learning key",
  il tasto K1, il potenziometro con la scritta **"Closing Obstruction"** e una
  freccia "+", il DIP rosso a 4 posizioni numerate 1-4.
- Posizione dei DIP: **mi sembra** che la levetta 1 e la 4 siano da una
  parte, la 2 e la 3 dall'altra. Ma la scritta "ON" non si vede, quindi
  **non so quale lato sia ON**.

**Schema del motore** (foto 3): una morsettiera a 7 posizioni con terra
("Ground line"), comune ("Public line"), due fili di direzione ("Direction
line") e tre fili del finecorsa ("Limit line"). Tutti i fili del motore
sono **a 230 V ⚡**, tranne forse il finecorsa: non è indicato.

## 4.3 Cosa non si legge o non è chiaro

1. **Il morsetto verde a tre posizioni.** Tu leggi "open / stop / close".
   Nel manuale, invece, la didascalia "Lights AC220V" (lampeggiante a 230 V)
   punta proprio a un morsetto verde. Le possibilità sono due:
   - la tua scheda è una versione diversa da quella del manuale;
   - sulla scheda ci sono due morsetti verdi.

   **Finché non è chiarito, va trattato come se avesse 230 V** (ipotesi H4).
2. **Ingressi "chiudi" e "stop".** Nella morsettiera a 5 posizioni descritta
   dal manuale **non ci sono** ingressi separati per chiudere e fermare: solo
   "Open" (2-4) e "cycle" (2-5). Gli ingressi separati esistono solo se il
   morsetto verde è davvero "open / stop / close" a bassa tensione.
3. **"test" o "cycle"?** Tu leggi "test" sul quinto morsetto, il manuale
   scrive "cycle". Potrebbe essere lo stesso ingresso, cioè apri → stop →
   chiudi → stop a ogni impulso. **Non useremo questo ingresso per il
   tetto**: con un solo tasto non si sa mai in che verso partirà.
4. **Il potenziometro.** Sulla scheda c'è scritto "Closing Obstruction",
   cioè sensibilità agli ostacoli. Il manuale (foto 1, punto 4) dice invece
   che il potenziometro regola la **richiusura automatica** ("Turn
   counterclockwise to minimum close automatic door closing"). Le due
   informazioni sono in contrasto.
5. **Richiusura automatica spenta.** Il manuale elenca solo tre combinazioni
   dei DIP 2-3, tutte con un tempo (5, 10, 20 s). Non dice che 2 OFF + 3 OFF
   la disattiva. **È fondamentale verificarlo**: una richiusura automatica
   attiva chiuderebbe il tetto da sola dopo l'apertura.
6. **Che cosa succede dopo un blackout.** Il manuale dice "the controller is
   turned on after the power is off. First of all, execute the turn on"
   (punto 2) e "the door must be pushed to the middle during installation"
   (punto 1). Sembra significare che, al ritorno della corrente, la scheda
   non sa dov'è il cancello e il primo comando potrebbe muoverlo in un verso
   inatteso. Da verificare al collaudo, con il tetto sorvegliato.
7. **Cancellazione dei codici.** Il manuale dice 12 secondi, la scheda dice
   8. Non ha importanza per noi, ma conferma che manuale e scheda non
   corrispondono del tutto.
8. **Pagine mancanti.** Mancano la pagina successiva a "Power supply test" e
   le pagine sui morsetti e sulle fotocellule.
9. **Ingresso "Photo".** Non sappiamo se è normalmente chiuso o aperto, né
   se oggi c'è un ponticello.
10. **Sigla ABM241.** Non ho trovato documentazione pubblica di questa
    scheda, solo di schede simili. Per questo non uso nessuna informazione
    "presa da internet" come certa.

## 4.4 Le due strade possibili (decideremo con i dati)

Lo sketch dell'Arduino è **lo stesso** nei due casi: tre relè, cioè tre
"pulsanti" APRI, CHIUDI e STOP. Cambiano solo i fili e qualche valore in
`arduino/tetto/configurazione_scheda.h`.

**Strada A — ingressi della scheda (preferita).** Vale **se** il morsetto
verde risulta "open / stop / close" a bassa tensione:

- i tre relè vanno ai tre ingressi;
- il comune va al morsetto indicato dalle misure, probabilmente il 2 (GND).

**Strada B — telecomando di scorta in parallelo.** Vale **se** il morsetto
verde è il lampeggiante a 230 V, oppure se gli ingressi non sono
utilizzabili:

- si abbina alla scheda un **secondo telecomando** uguale;
- lo si apre e si collegano i relè ai tasti Open, close e Stop.

Il telecomando funziona a pila (12 V), quindi è a bassa tensione e
isolato dalla scheda. Lo svantaggio è che passa via radio. Se un comando
non arriva, però, l'Arduino se ne accorge ("il tetto non è partito") e
blocca tutto.

In **nessun caso** si userà l'ingresso "cycle" passo-passo, e in nessun caso
l'Arduino verrà collegato direttamente ai morsetti della scheda.

## 4.5 Quale relè

- **Strada A.** Gli ingressi sono probabilmente alimentati dai 15 V della
  scheda, con correnti di pochi milliampere. Il modulo a 4 relè della lista
  va bene. Per correnti così piccole sono ancora più affidabili i relè con
  **contatti dorati** (vedi la lista della spesa, voce A3).
- **Strada B.** I tasti del telecomando lavorano con correnti piccolissime:
  anche qui vanno bene, e anzi sono preferibili, i contatti dorati.
- **In entrambi i casi:** se le misure dessero più di 30 V, o tensione
  alternata, ci si ferma e decide l'elettricista.

# 4. La scheda del cancello: come verificarla e quali relè usare

> ⏳ **Questa parte resta in sospeso finché non vedo la foto della scheda.**
> Qui sotto c'è come prepararti. **Non collegare ancora niente** alla scheda.

> ⚠️ La scheda del cancello è alimentata a **230 V**: sulla stessa scheda
> ci sono morsetti a bassa tensione (i comandi) e morsetti a 230 V
> (alimentazione, motore, a volte il lampeggiante). Fai le misure "a scheda
> accesa" **solo con un elettricista presente**. Se non te la senti, chiedi
> a lui di fare le misure del punto 3 e di scriverti i valori.

## 4.1 Cosa mi serve (da raccogliere con calma)

1. **Foto della scheda intera**, dall'alto, ben illuminata.
2. **Foto ravvicinata della morsettiera**, cioè della fila di viti dove si
   collegano i fili, con **le scritte leggibili**. Se le scritte sono sotto i
   fili, fotografa da più angolazioni.
3. **Foto di eventuali interruttorini (DIP switch) e manopoline (trimmer)**
   con le loro scritte.
4. **Foto dell'etichetta** con modello e dati del motore e della scheda.
5. Il **manuale**, se ce l'hai, anche in inglese o come foto delle pagine.
   Spesso è un PDF che il venditore manda su richiesta.
6. Il **telecomando**: quanti tasti ha e cosa fa ciascuno?
7. I risultati delle misure del punto 3, se riesci a farle.

## 4.2 Come leggere la morsettiera (senza toccare nulla)

Sulle schede per cancelli scorrevoli i morsetti dei comandi hanno di solito
sigle come queste. Le elenco solo perché tu sappia cosa cercare: **non so
ancora quali ha la tua scheda.**

| Sigle possibili | Significato abituale |
|---|---|
| OPEN, APRE, AP | ingresso comando "apri" |
| CLOSE, CHIUDE, CH | ingresso comando "chiudi" |
| STOP | ingresso "stop" (spesso **normalmente chiuso**: va tenuto collegato a COM, e lo stop avviene "aprendo" il collegamento) |
| SBS, PP, STEP, P/P, K | "passo-passo": un solo tasto che fa apri → stop → chiudi → stop… |
| PED | apertura pedonale (parziale) |
| COM, GND, ⏚ | comune dei comandi |
| PHO, FOTO, FT | fotocellule |
| LAMP, FLASH | lampeggiante (spesso **230 V!**) |
| 12V, 24V, +, − | uscita di alimentazione per accessori |
| L, N, AC 220V | alimentazione a **230 V ⚡** |
| M, MOT, U V W, CAP | motore e condensatore, a **230 V ⚡** |

## 4.3 Le misure (tensione e tipo degli ingressi)

**Attrezzo:** multimetro digitale ("tester").

1. Ruota la manopola del tester su **V con la linea dritta e i puntini**
   (tensione continua, "V⎓" o "DCV"), portata **20 V** se il tuo tester non
   sceglie da solo la portata.
2. Puntale **nero** nella presa "COM" del tester, puntale **rosso** nella
   presa "V".
3. A scheda accesa (con l'elettricista presente), appoggia:
   - il puntale nero sul morsetto **COM** (o GND) **dei comandi**;
   - il puntale rosso, uno alla volta, sui morsetti **OPEN**, **CLOSE**,
     **STOP** (o quelli che hai trovato).
4. Annota il numero che leggi per ciascuno.

**Come interpretare i numeri:**

| Leggi | Significa | Relè adatti |
|---|---|---|
| Tra 3 e 30 V circa | Ingresso a bassa tensione, a "contatto pulito": basta unire quel morsetto con COM per dare il comando. È il caso più comune. | Il modulo relè della lista (A3) va bene. |
| 0 V su STOP, ma un valore su OPEN/CLOSE | Probabilmente lo STOP è "normalmente chiuso" e c'è già un ponticello. | Si userà il contatto NC del relè 3. |
| Più di 30 V, o valori alternati | **Fermati.** Potrebbe essere un ingresso ad alta tensione. | Niente modulo economico: decide l'elettricista. |

**Facoltativo, solo se hai letto 3-30 V e con l'elettricista:** prova il
comando a mano. Togli prima il motore dalla cremagliera con la **chiave di
sblocco manuale** (così il motore gira "a vuoto" e il tetto non si muove).
Poi unisci per un attimo, con un filo isolato, il morsetto OPEN con COM:
il motore deve partire. Ripeti con CLOSE e con STOP.

## 4.4 Impostazioni della scheda da controllare (con il manuale)

| Funzione | Come deve essere | Perché |
|---|---|---|
| **Richiusura automatica** (auto-close, "TCA", "pausa") | **DISATTIVATA** | Altrimenti il tetto si richiude da solo dopo qualche secondo! |
| Rilevamento ostacoli / sensibilità di forza | Attivo, tarato basso | Se qualcosa blocca il tetto, il motore si ferma. |
| Finecorsa della scheda (magneti o molle sulla cremagliera) | Regolati in modo da fermare il tetto **un po' oltre** i nostri finecorsa | Sono la protezione meccanica contro il fine binario. |
| Tempo di lavoro | Poco più della corsa reale | Seconda protezione "a tempo", indipendente dall'Arduino. |
| Fotocellule | Se non le usi, l'ingresso va ponticellato come dice il manuale | Altrimenti la scheda può rifiutare la chiusura. |
| Rallentamento a fine corsa (soft stop) | Se c'è, attivo | Meno urti sul tetto. |

## 4.5 Quale relè: il ragionamento

- I relè del modulo A3 sopportano fino a 10 A a 250 V / 30 V continui. Per
  "premere un tasto" a 12-24 V con pochi milliampere sono più che
  sufficienti.
- Con correnti piccolissime, i contatti normali (in argento) a volte nel
  tempo diventano poco affidabili. Se al collaudo un comando ogni tanto non
  viene ricevuto, si passa a relè con **contatti dorati**, adatti ai piccoli
  segnali.
- Il modulo ha gli **optoisolatori**, cioè una separazione elettrica tra
  Arduino e relè. Protegge l'Arduino dai disturbi che arrivano dalla scheda
  del cancello.

## 4.6 E se la scheda fosse "difficile"?

Se la morsettiera non avesse ingressi separati per apri e chiudi (solo
"passo-passo"), o se fosse illeggibile, c'è un'alternativa che funziona
**sempre**: collegare i relè **in parallelo ai tasti di un telecomando di
scorta**. Il telecomando va a pila, a 3-12 V, ed è quindi sicuro. Lo si apre
e si saldano due fili ai capi di ogni tasto. Se servirà, ti guiderò passo
passo.

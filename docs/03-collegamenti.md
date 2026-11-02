# 3. Schema dei collegamenti

> ⚠️ **Tutto ciò che riguarda i 230 V lo fa un elettricista.** Le parti a
> 230 V sono segnate con ⚡. Tutti gli altri collegamenti sono a bassa
> tensione (5 V e 12 V), non pericolosi, e li puoi fare tu, **sempre a
> corrente staccata**.
>
> ⏳ = collegamento che dipende dalla scheda del cancello. Lo completeremo
> dopo la foto dei morsetti.

## 3.1 Regola generale per tutti i sensori

Ogni sensore è un **interruttore** (un "contatto") collegato tra un pin
dell'Arduino e il GND, cioè il negativo. L'Arduino tiene il pin a 5 V con
una resistenza interna ("pull-up").

- Quando il contatto si **chiude**, il pin va a 0 V e l'Arduino legge "attivo".
- Quando il contatto è **aperto**, oppure **il filo è tagliato**, il pin
  resta a 5 V e l'Arduino legge "non attivo".

Abbiamo scelto i collegamenti in modo che un filo tagliato porti sempre
dalla parte **sicura**:

| Sensore | Contatto chiuso significa | Se il filo si taglia l'Arduino crede... | Conseguenza |
|---|---|---|---|
| Finecorsa APERTO | tetto tutto aperto | "non è aperto" | il movimento si ferma per tempo scaduto: nessun danno |
| Finecorsa CHIUSO | tetto tutto chiuso | "non è chiuso" | Ekos non crede mai che il tetto sia chiuso quando non lo è |
| Montatura parcheggiata | montatura in parcheggio | "non parcheggiata" | il tetto non si muove |
| Emergenza (contatto NC) | tutto normale | "emergenza" | il tetto non si muove |
| Pioggia | asciutto | "piove" | condizioni non sicure |

## 3.2 Arduino del tetto (Uno)

```
                         ARDUINO UNO (con morsettiera a vite)
                    ┌──────────────────────────────────────────┐
   alimentatore ───►│ presa 12 V                               │
   12 V DC          │                                          │
   PC ──── USB ────►│ presa USB                                │
                    │                                          │
                    │ D4  ─────────────────────► IN1  ┐        │
                    │ D7  ─────────────────────► IN2  │ MODULO │
                    │ D8  ─────────────────────► IN3  │ 4 RELÈ │
                    │ D12 ─────────────────────► IN4  │        │
                    │ 5V  ─────────────────────► VCC  │        │
                    │ GND ─────────────────────► GND  ┘        │
                    │                                          │
                    │ A0 ──── finecorsa APERTO ────┐           │
                    │ A1 ──── finecorsa CHIUSO ────┤           │
                    │ A2 ──── reed AR ─ reed DEC ──┤ (in serie)│
                    │ A3 ──── relè pioggia (contatto 2) ───────┤
                    │ A4 ──── emergenza (contatto NC) ─────────┤
                    │ D2 ──── reed movimento (facoltativo) ────┤
                    │ GND ─────────────────────────┘ ritorno comune
                    └──────────────────────────────────────────┘
```

**In parole:**

1. **Relè.** I pin D4, D7, D8 e D12 dell'Arduino vanno agli ingressi IN1,
   IN2, IN3 e IN4 del modulo relè. Il 5V e il GND dell'Arduino vanno a VCC e
   GND del modulo. Lascia il ponticello "JD-VCC" al suo posto.
   - Relè 1 = **APRI**
   - Relè 2 = **CHIUDI**
   - Relè 3 = **STOP**
   - Relè 4 = di riserva, non collegato
2. **Finecorsa APERTO.** Due fili: uno al morsetto **COM** del finecorsa,
   l'altro al morsetto **NO** (o "NA", normalmente aperto). All'Arduino uno
   va su A0 e l'altro su GND.
3. **Finecorsa CHIUSO.** Uguale, ma su A1 e GND.
4. **Montatura parcheggiata.** I due contatti magnetici (uno per asse) si
   collegano **uno dopo l'altro** (in serie): A2 → contatto AR → contatto
   DEC → GND. Il circuito è chiuso solo se entrambi gli assi sono in
   parcheggio. I magneti si fissano sulla parte mobile della montatura, i
   contatti su quella fissa, in modo che si trovino affiancati (a meno di 1
   cm) solo nella posizione di parcheggio.
5. **Emergenza.** Il contatto NC "libero" del pulsante di emergenza va tra
   A4 e GND.
6. **Pioggia.** Il secondo contatto del relè di appoggio della pioggia
   (vedi 3.4) va tra A3 e GND.
7. **Sensore di movimento (facoltativo, consigliato).** Un contatto
   magnetico reed fisso, accanto al binario, tra D2 e GND, e dei piccoli
   magneti fissati sul tetto ogni 30 cm circa. Ogni magnete che passa
   chiude il contatto per un attimo: così l'Arduino sa se il tetto si muove
   davvero, anche a metà corsa. Si attiva con `USA_SENSORE_MOVIMENTO 1` in
   `configurazione_scheda.h`. Se la distanza tra i magneti cambia, va
   ricalcolato `FINESTRA_MOVIMENTO_MS`.
8. **Alimentazione.** L'alimentatore 12 V va nella presa tonda
   dell'Arduino, la USB al PC. Con entrambi collegati, l'Arduino usa
   automaticamente i 12 V e resta acceso anche se il PC si spegne.

### Relè → scheda del cancello ⏳ (PROVVISORIO: NON COLLEGARE ANCORA)

> Questa sezione verrà completata quando avremo i dati dei controlli
> ([10-controlli-senza-corrente.md](10-controlli-senza-corrente.md)). Le
> ipotesi su cui si basa sono in
> [09-ipotesi-da-verificare.md](09-ipotesi-da-verificare.md).

**Il principio.** Ogni relè ha tre morsetti:

- **COM** (comune);
- **NO** (normalmente aperto);
- **NC** (normalmente chiuso).

Per imitare un pulsante si usano COM e NO: quando il relè scatta, COM e NO
si collegano per mezzo secondo, come un dito che preme il tasto. **Dalla
parte della scheda arrivano solo i fili dei contatti dei relè.** L'Arduino
non è mai collegato ai morsetti della scheda.

Tra la scatola dei relè e la scheda usa un **cavo separato** da quelli della
corrente e del motore. Lo chiede anche il manuale della scheda: "separate the
power line from the control line".

**Strada A — ingressi della scheda** (solo se il morsetto verde risulta
"open / stop / close" a bassa tensione: ipotesi H4, H5, H8):

```
   RELÈ 1 (APRI)    COM ── comune ⏳ (ipotesi: morsetto 2 "GND")      NO ── ingresso "open" ⏳
   RELÈ 2 (CHIUDI)  COM ── comune ⏳                                  NO ── ingresso "close" ⏳
   RELÈ 3 (STOP)    COM ── comune ⏳                                  NO (o NC, H8) ── ingresso "stop" ⏳
```

**Strada B — telecomando di scorta in parallelo ai tasti** (se il morsetto
verde è il lampeggiante a 230 V):

```
   RELÈ 1 (APRI)    COM / NO ── ai due capi del tasto "Open"  (▲) del telecomando di scorta
   RELÈ 2 (CHIUDI)  COM / NO ── ai due capi del tasto "close" (▼)
   RELÈ 3 (STOP)    COM / NO ── ai due capi del tasto "Stop"  (■)
```

(Come aprire il telecomando e dove saldare i fili lo spiegherò se si
sceglie questa strada.)

**In nessun caso:**

- si collega qualcosa al morsetto verde prima delle misure dell'elettricista
  (potrebbe essere a 230 V);
- si usa l'ingresso "cycle"/"test" (passo-passo);
- si collega l'Arduino direttamente alla scheda.

## 3.3 Arduino della stazione meteo (Nano)

```
                     ARDUINO NANO
                 ┌───────────────────┐
  PC ── USB ────►│ USB               │
                 │                   │        MLX90614 (modulo GY-906)
                 │ 5V  ──────────────┼──────► VIN
                 │ GND ──────────────┼──────► GND
                 │ A4  ──────────────┼──────► SDA
                 │ A5  ──────────────┼──────► SCL
                 │                   │
                 │ D2  ──────────────┼──────► anemometro (filo 1)
                 │ GND ──────────────┼──────► anemometro (filo 2)
                 │                   │
                 │ D4  ──────────────┼──────► relè pioggia, contatto 1 (morsetto 14)
                 │ GND ──────────────┼──────► relè pioggia, contatto 1 (morsetto 11)
                 └───────────────────┘
```

**In parole:**

1. **MLX90614.** È il sensore a infrarossi, con 4 fili: VIN al 5V del Nano,
   GND al GND, SDA ad A4, SCL ad A5. Il cavo tra Nano e sensore deve essere
   **il più corto possibile: al massimo 2 metri**, meglio se schermato. Per
   questo l'Arduino Nano va in una scatola stagna vicino al sensore. Dal Nano
   al PC basta poi un cavo USB, lungo al massimo 5 metri: per distanze
   maggiori serve una prolunga USB "attiva".
2. **Anemometro.** Ha due fili: D2 e GND. Il Misol ha un connettore
   telefonico RJ11 e i due fili utili sono di solito **i due centrali**.
   Verificalo col tester in modalità "continuità" (quella che fa il bip):
   girando lentamente le coppette il bip deve comparire e scomparire.
3. **Pioggia.** Il primo contatto del relè di appoggio va a D4 e GND (vedi 3.4).

## 3.4 Sensore di pioggia: un sensore, due Arduino

Il sensore di pioggia serve **sia** al programma meteo **sia** all'Arduino
del tetto. Così il tetto può chiudersi per pioggia anche se il PC è morto,
quando attiverai quella funzione. Per non collegare tra loro i due Arduino si
usa un relè "di appoggio" con due contatti separati, il componente B5 della
lista della spesa.

```
  +12 V ──┬────────────────────────► RG-11 alimentazione (+)
          │
          └──► RG-11 contatto "C" ─── RG-11 contatto chiuso-quando-ASCIUTTO ──┐
                                                                             │
                                          bobina relè B5 (morsetti A1 - A2) ◄─┘
                                                     │
  0 V ───────────────────────────────────────────────┴──► RG-11 alimentazione (−)

  Relè B5:  contatto 1 (morsetti 11-14) ──► Arduino METEO  D4 / GND
            contatto 2 (morsetti 21-24) ──► Arduino TETTO  A3 / GND
```

**Come funziona:**

- **Tempo asciutto.** Il contatto dell'RG-11 è chiuso, la bobina del relè
  B5 riceve corrente e i suoi due contatti NO (11-14 e 21-24) sono chiusi.
  Entrambi gli Arduino leggono "asciutto".
- **Pioggia.** L'RG-11 apre il contatto, il relè B5 si diseccita e i
  contatti si aprono. Entrambi gli Arduino leggono "piove".
- **Corrente a 12 V mancante o filo tagliato.** Il risultato è lo stesso
  della pioggia: "piove". È la parte sicura.

Quale contatto dell'RG-11 è "chiuso quando è asciutto" si verifica col tester
(vedi il collaudo, fase A). I +12 V e gli 0 V vengono dallo stesso
alimentatore A7 dell'Arduino del tetto.

**Impostazione dell'RG-11:** con i piccoli interruttori ("DIP switch")
all'interno, scegli la modalità di **rilevazione gocce** ("drop detector"),
seguendo il manuale Hydreon. In questa modalità il contatto scatta dopo poche
gocce.

## 3.5 Parte a 230 V ⚡ (solo elettricista)

```
  Quadro elettrico ⚡
     │
     ├─ interruttore magnetotermico-differenziale dedicato all'osservatorio ⚡
     │
     ├──► Gruppo di continuità (UPS) ──► PC
     │                               └──► alimentatore 12 V (A7)
     │
     └──► PULSANTE DI EMERGENZA (contatto NC 1) ⚡ ──► scheda del cancello (L/N) ⚡
                       │
                       └── contatto NC 2 (bassa tensione) ──► Arduino tetto A4 / GND
```

- Il pulsante di emergenza **toglie corrente alla scheda del cancello**,
  cioè al motore. È la protezione che funziona anche se Arduino e PC
  impazziscono.
- L'elettricista sceglie come interrompere l'alimentazione: direttamente,
  o tramite un contattore se la corrente del motore lo richiede. Deve però
  lasciare **un contatto NC libero, a bassa tensione**, per l'Arduino.
- Il motore da 370 W, la scheda e la messa a terra della struttura metallica
  vanno verificati dall'elettricista, insieme all'impianto dell'osservatorio.

## 3.6 Consigli pratici

- **Distanza dai cavi del motore.** Tieni i cavi dei sensori (bassa
  tensione) lontani da quelli del motore e del 230 V, ad almeno 20-30 cm,
  oppure in canaline separate.
- **Etichette.** Metti un'etichetta a ogni cavo, alle due estremità.
- **Fotografie.** Fai una foto di ogni collegamento finito e conservala:
  serve per la manutenzione.
- **Schermatura.** La calza dei cavi schermati va collegata al GND solo dal
  lato Arduino.

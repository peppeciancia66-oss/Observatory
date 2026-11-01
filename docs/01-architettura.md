# 1. Architettura del sistema

## In breve: cosa è collegato a cosa

```
                         PC dell'osservatorio (Linux Mint)
                         KStars / Ekos  +  INDI
        ┌───────────────────────────┬──────────────────────────────────┐
        │ driver "RollOff ino"      │ driver "Weather Safety Proxy"    │
        │ (il tetto)                │ (il meteo)                       │
        └─────────┬─────────────────┴───────────┬──────────────────────┘
                  │ USB                         │ ogni 60 s esegue
                  │ /dev/osservatorio-tetto     │ meteo/weather_status.py
                  ▼                             ▼
        ┌─────────────────────┐      programma meteo (servizio Python
        │  ARDUINO UNO        │      "osservatorio-meteo", sempre acceso)
        │  sketch tetto.ino   │                 ▲
        └──┬───────┬──────┬───┘                 │ USB /dev/osservatorio-meteo
           │       │      │            ┌────────┴─────────┐
           │       │      │            │  ARDUINO NANO    │
           │       │      │            │  sketch meteo.ino│
           │       │      │            └──┬──────┬─────┬──┘
           │       │      │               │      │     │
           │       │      │         MLX90614  anemometro  sensore pioggia
           │       │      │         (nuvole)  (vento)     (Hydreon RG-11)
           │       │      │                                   │
           │       │      └── ingressi: finecorsa APERTO/CHIUSO,
           │       │          MONTATURA PARCHEGGIATA, EMERGENZA,
           │       │          PIOGGIA ◄───────────────────────┘ (secondo contatto)
           │       │
           │   modulo 4 relè ── APRI / CHIUDI / STOP ──► scheda del cancello SHZICMY
           │                                               │
           └── LED di stato                                ▼
                                                     motore 370 W + cremagliera
```

## Perché Arduino e non Raspberry Pi

| | Arduino Uno | Raspberry Pi |
|---|---|---|
| Driver INDI pronto | **Sì** ("RollOff ino" è fatto apposta) | Bisognerebbe scrivere o adattare un driver |
| Avvio | 1 secondo | 30-60 secondi |
| Interruzioni di corrente | Nessun problema | La scheda SD si può rovinare |
| Manutenzione | Nessuna (niente sistema operativo da aggiornare) | Un secondo computer Linux da curare |
| Funziona anche se il PC si blocca | **Sì**: i timer di sicurezza sono sull'Arduino | Sì, ma con molta più complessità |

Il PC c'è già e INDI gira già lì. Un Raspberry Pi aggiungerebbe solo un
secondo computer da mantenere.

## Perché due Arduino separati

- Se si guasta la stazione meteo (che sta all'aperto, con fulmini, umidità e
  cavi lunghi), il tetto continua a funzionare, e viceversa.
- Il programma del tetto resta piccolo e facile da verificare.

## Perché "Weather Safety Proxy" e non "Weather Watcher"

- **Weather Safety Proxy** chiede a un nostro script "è sicuro sì o no, e
  perché". Tutte le soglie restano in **un solo file** (`meteo.ini`), che
  modifichi con un editor di testo. In Ekos vedi "sicuro/non sicuro" e il
  motivo, scritto in italiano.
- Con **Weather Watcher**, invece, le soglie si impostano dentro Ekos. Inoltre
  la modalità taratura e la regola "sensore guasto = non sicuro" andrebbero
  costruite intorno al driver.

## Che cosa decide chi

| Decisione | Chi la prende | Dove si configura |
|---|---|---|
| Fermare il motore (tempo massimo, sensori, PC muto, emergenza) | **Arduino del tetto**, da solo | `arduino/tetto/tetto.ino`, sezione CONFIGURAZIONE |
| Rifiutare il movimento se la montatura non è parcheggiata | **Arduino del tetto** (sensore fisico) **e** Ekos ("Mount Policy") | Sensore sulla montatura + opzioni del driver |
| Dire se il meteo è sicuro | Programma meteo sul PC | `~/.config/osservatorio/meteo.ini` |
| Chiudere il tetto quando il meteo diventa non sicuro | **Ekos** (modulo Observatory), **disattivato di default** | Ekos → Observatory → Weather |
| Chiudere il tetto per pioggia se il PC è morto | Arduino del tetto, **disattivato di default** | `CHIUSURA_AUTONOMA_PIOGGIA` in `tetto.ino` |

## Il controllo "montatura parcheggiata": due protezioni

1. **Protezione di Ekos ("Mount Policy").** Ekos sa se la montatura è
   parcheggiata. Nelle opzioni del driver del tetto imposti
   **Mount Policy = Mount locks**. Attenzione: verificando il codice di INDI ho
   visto che questa opzione blocca solo la **chiusura**, non l'apertura.
2. **Protezione fisica (la più importante).** Un sensore magnetico sulla
   montatura dice all'Arduino "sono nella posizione di parcheggio". Se il
   sensore non lo conferma, l'Arduino **rifiuta sia l'apertura sia la
   chiusura**. In Ekos si accende la luce rossa "Roof Lock". Se la montatura
   esce dal parcheggio mentre il tetto si muove, l'Arduino ferma subito il
   motore. Questa protezione funziona anche se Ekos ha un'informazione
   sbagliata, per esempio una montatura spostata a mano a driver spento.

## Cosa succede se il PC si blocca o cade il collegamento

| Situazione | Cosa succede | Protezione |
|---|---|---|
| Il PC si blocca **mentre il tetto si muove** | L'Arduino non riceve più domande dal driver e dopo 15 s preme STOP. Il tetto resta fermo, bloccato, fino al tuo controllo. | Già nello sketch |
| Il cavo USB si stacca durante il movimento | Come sopra. Se l'Arduino era alimentato solo dalla USB, si spegne e i relè tornano a riposo: il tetto si ferma ai finecorsa della scheda del cancello. | Alimentatore 12 V separato per l'Arduino (consigliato) |
| Il programma dell'Arduino si pianta | Il "watchdog" lo riavvia in 2 s, con tutti i relè a riposo. | Già nello sketch |
| Il PC si blocca **a tetto aperto e fermo** | Il tetto **resta aperto**. Se arriva la pioggia, nessuno lo chiude. | Vedi sotto |
| Manca la corrente a 230 V | Il motore non può muoversi. Il tetto resta com'è. | Gruppo di continuità (UPS) per il PC; batteria tampone per la scheda del cancello, se prevista dal modello |
| Il programma meteo si blocca | Dopo 90 s lo script risponde a Ekos NON SICURO e il servizio viene riavviato da solo. | Già nel software |

**Soluzioni per il caso "tetto aperto, PC bloccato, arriva la pioggia":**

1. **Chiusura autonoma per pioggia** (già scritta, **disattivata**). L'Arduino
   del tetto legge anche lui il sensore di pioggia. Se il PC tace da 2 minuti
   e piove da 30 secondi, chiude il tetto **solo se** la montatura risulta
   parcheggiata. Si attiva cambiando `CHIUSURA_AUTONOMA_PIOGGIA` da 0 a 1,
   **solo dopo il collaudo completo**.
2. **Watchdog di INDI** (facoltativo). È un driver chiamato "WatchDog"
   che parcheggia montatura e tetto se Ekos smette di funzionare. Copre il
   caso in cui si chiude Ekos, ma non quello in cui si blocca tutto il PC.
3. **Gruppo di continuità (UPS)** per il PC e per l'alimentatore dell'Arduino.
4. **Abitudine:** a fine sessione, se non usi l'automazione, chiudi il tetto.

Limite da conoscere: se il PC si blocca **mentre la montatura punta il
cielo**, il tetto non si chiude, né per comando né da solo. Il sensore di
parcheggio lo impedisce, ed è giusto così: meglio un telescopio bagnato che
un tetto che lo colpisce. Per questo conviene parcheggiare la montatura
quando non si osserva.

## Due particolarità del driver "RollOff ino" (trovate leggendo il suo codice)

1. Il driver riconosce "acceso" cercando la parola `ON` dentro la risposta
   dell'Arduino. Per questo i messaggi di errore dello sketch sono tutti in
   minuscolo.
2. Se l'Arduino rifiuta un comando, il driver crede comunque che sia stato
   accettato e mostra "in movimento" fino al proprio timeout. Per questo
   ogni rifiuto viene comunicato **prima**, con la luce "Roof Lock" (stato
   `LOCKED`). Il driver, vedendola, non invia nemmeno il comando e scrive
   nel registro "Roof is externally locked".

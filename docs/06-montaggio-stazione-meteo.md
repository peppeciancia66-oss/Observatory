# 6. Montaggio della stazione meteo e protezione dalle intemperie

## 6.1 Sensore a infrarossi MLX90614 (nuvole)

**Come funziona.** Il sensore misura la temperatura "vista" verso l'alto.

- **Cielo sereno:** il cielo è freddissimo (−20 / −40 °C) anche quando
  l'aria è a +10 °C.
- **Cielo coperto:** le nuvole sono quasi alla temperatura dell'aria.

La **differenza** tra temperatura del cielo e temperatura dell'aria (letta
dallo stesso modulo) dice quindi quanto è coperto il cielo.

**Montaggio:**

1. **Posizione.** Il sensore guarda **dritto verso lo zenit**. Il modello
   BAA ha un campo visivo molto ampio (circa 90°): non devono esserci alberi,
   tetti o pareti entro 45° dalla verticale.
2. **Scatola.** Metti il sensore in una piccola scatola stagna, con un foro
   di 12-15 mm sopra di esso.
3. **Finestrella.** Chiudi il foro con un pezzetto di **pellicola
   trasparente per alimenti in polietilene**, ben tesa e fissata con un
   anello o del nastro. Il vetro e il plexiglass **bloccano** l'infrarosso:
   non vanno bene. La pellicola invece lo lascia passare e tiene fuori acqua
   e insetti. Sostituiscila ogni 2-3 mesi.
4. **Gocce.** Inclina leggermente la scatola, di 5° al massimo, perché la
   pioggia non ristagni sulla pellicola. Una goccia o la rugiada sulla
   finestrella fanno sembrare il cielo "nuvoloso": è un errore dalla parte
   sicura.
5. **Sole.** Di giorno il sole falsa le misure. Non importa: di giorno non si
   osserva, e il riepilogo della taratura usa solo le ore notturne.
6. **Cavo.** Al massimo 2 metri fino all'Arduino Nano.

## 6.2 Anemometro

1. **Altezza.** Sul palo, **il più in alto possibile**: idealmente 1-2 m
   sopra il colmo del tetto dell'osservatorio e lontano da alberi e muri, che
   creano vortici.
2. **Orientamento.** L'asse delle coppette deve essere verticale: usa una
   livella.
3. **Cavo.** Fallo scendere lungo il palo con una piccola curva verso il
   basso ("goccia") prima di entrare nella scatola, così l'acqua non scorre
   dentro.
4. **Connettore.** Il Misol ha un connettore RJ11 (telefonico) che non è
   stagno. Mettilo **dentro** la scatola, oppure proteggilo con guaina
   termorestringente.

## 6.3 Sensore di pioggia Hydreon RG-11

1. **Esposizione.** Montalo **in piano**, con la cupola verso l'alto, in
   posizione libera, non sotto gronde o rami.
2. **Pulizia.** Una volta al mese pulisci la cupola con acqua e un panno
   morbido. Le ragnatele e la sporcizia possono dare falsi allarmi.
3. **Modalità.** Va impostata con i DIP switch interni: rilevazione gocce
   ("drop detector"), vedi `03-collegamenti.md`.
4. **Pressacavo.** Il cavo entra dal pressacavo del sensore, ben stretto.

## 6.4 Scatola dell'Arduino Nano

1. **Tipo di scatola.** Stagna, IP65, con i cavi che entrano **dal basso**
   attraverso pressacavi.
2. **Condensa.** Metti dentro una bustina di gel di silice (quelle delle
   scarpe) e cambiala ogni tanto.
3. **Posizione.** Mettila all'ombra, se possibile: il sole diretto la
   scalda troppo.
4. **Fori di drenaggio.** Un piccolo foro nel punto più basso, oppure un
   tappo "anticondensa" traspirante, evita che l'acqua di condensa si
   accumuli.

## 6.5 Fulmini e disturbi

- I cavi lunghi all'aperto raccolgono disturbi. Il programma meteo scarta
  le misure rovinate e, se non arrivano dati validi, segnala **guasto** e
  dichiara **non sicuro**.
- L'**isolatore USB** (B10) tra il Nano e il PC protegge il PC. Costa poco
  rispetto al PC.
- Durante i temporali forti, se possibile, è buona abitudine staccare il
  cavo USB della stazione meteo. Il programma segnalerà "nessun dato" e
  quindi "non sicuro": è il comportamento giusto.

## 6.6 Manutenzione mensile (5 minuti)

- [ ] Spruzza acqua sul sensore di pioggia: entro pochi secondi lo stato
      deve diventare "pioggia" (controlla con
      `watch -n 2 cat ~/osservatorio-meteo/stato_meteo.json`).
- [ ] Gira a mano le coppette dell'anemometro: il vento non deve restare a zero.
- [ ] Controlla la pellicola sopra l'MLX90614: deve essere integra e pulita.
- [ ] Pulisci la cupola del sensore di pioggia.
- [ ] Guarda `~/osservatorio-meteo/meteo.log` e cerca le parole "GUASTO" o "scartata".

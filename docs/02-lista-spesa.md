# 2. Lista della spesa

I prezzi sono **indicativi** e vanno verificati al momento dell'acquisto.
Dove scrivo "cerca:", sono le parole da usare su Amazon, eBay o nei negozi
di elettronica (in Italia, per esempio: Futura Elettronica, RS Components,
Mouser, Distrelec).

Le voci segnate con **⏳** dipendono dalla scheda del cancello. **Non
comprarle** prima che io abbia visto la foto dei morsetti.

## A. Tetto

| # | Componente | Modello consigliato | Q.tà | Prezzo indicativo | Note |
|---|---|---|---|---|---|
| A1 | Arduino Uno R3 **originale** | Arduino A000066 | 1 | 25 € | L'originale ha un numero di serie: Linux lo riconosce sempre (vedi `strumenti/crea_regola_udev.sh`). |
| A2 | Morsettiera a vite per Uno | cerca: "Arduino Uno screw terminal shield" | 1 | 8-12 € | Per collegare i fili con le viti, senza saldare. |
| A3 ⏳ | Modulo 4 relè 5 V con optoisolatori | cerca: "modulo 4 relè 5V optoisolato" (es. AZDelivery, Elegoo). Ancora meglio, per i piccoli segnali: 3-4 relè con **contatti dorati** (es. Finder 34.51.7.005.5010 o Omron G5V-1-5VDC) su una basetta con driver | 1 | 8-10 € (modulo) | Il manuale indica ingressi a bassa tensione (uscita accessori 15 V, 30 mA): basta un contatto pulito. **Da confermare con le misure (H3)**. |
| A4 | Finecorsa tetto aperto/chiuso | Finecorsa a leva con rotella IP65, 1NA+1NC. Economico: cerca "finecorsa ME-8108". Migliore: Omron D4N o Telemecanique XCK | 2 (+1 di scorta) | 5-30 € cad. | Si montano sul binario: la rotella viene premuta dal tetto a fine corsa. |
| A5 | Sensore "montatura parcheggiata" | Contatto magnetico reed da allarme, tipo "MC-38" (contatto chiuso con magnete vicino) | 2 | 3-5 € | Uno per asse (AR e DEC), collegati in serie: il contatto è chiuso solo se **entrambi** gli assi sono in parcheggio. |
| A6 | Pulsante di emergenza | A fungo, con ritenuta e sblocco a rotazione, **2 contatti NC**, in scatola IP65 (es. Schneider Harmony XALK, Eaton M22) | 1 | 25-45 € | Un contatto toglie corrente alla scheda del cancello (lo collega l'elettricista), l'altro va all'Arduino. |
| A7 | Alimentatore 12 V DC, 1 A | Spinotto 5,5×2,1 mm (centrale positivo) | 1 | 10 € | Tiene acceso l'Arduino anche se il PC si spegne. Alimenta anche il sensore di pioggia (B4). |
| A8 | Scatola per Arduino e relè | Scatola di derivazione IP55/IP65, circa 150×110×70 mm (es. Gewiss GW44206) + pressacavi | 1 | 10-15 € | |
| A9 | Cavo USB A-B ("da stampante") con ferrite | lunghezza 1-3 m | 1 | 5-8 € | |
| A10 | Cavo per i sensori | Cavo schermato da allarme 4×0,22 mm² | 20-30 m | 15 € | Per finecorsa, sensore della montatura, emergenza. |
| A11 | Gruppo di continuità (UPS) | 600-800 VA (es. APC Back-UPS, Eaton Ellipse) | 1 | 70-120 € | Per il PC e l'alimentatore A7. |
| A12 ⏳ | Batteria tampone per la scheda del cancello | Solo se il modello SHZICMY la prevede | 1 | - | Lo vediamo dalla foto o dal manuale. |
| A13 | Sensore di movimento (consigliato) | 1 contatto magnetico reed tipo "MC-38" + 15-20 piccoli magneti al neodimio (es. 10×3 mm) | 1 + magneti | 8 € | Montato accanto al binario, con i magneti sul tetto ogni 30 cm circa: dice all'Arduino se il tetto si muove davvero, anche a metà corsa. |
| A14 ⏳ | Telecomando di scorta (solo per la "strada B") | Uguale all'originale: 433 MHz, 4 tasti, chip EV1527, pila 27A 12 V | 1 | 8-12 € | Si abbina alla scheda e si collega ai relè al posto degli ingressi. Comprarlo solo se le verifiche indicano la strada B. |
| A15 | Cavo per i comandi relè → scheda | Cavo schermato 4×0,22 mm², **separato** da quelli di corrente e motore | 5-10 m | 5 € | Il manuale della scheda chiede di separare le linee di potenza da quelle di comando. |

## B. Stazione meteo

| # | Componente | Modello consigliato | Q.tà | Prezzo indicativo | Note |
|---|---|---|---|---|---|
| B1 | Arduino Nano | Originale Arduino A000005, oppure compatibile con chip CH340 | 1 | 22 € (orig.) / 5 € (comp.) | Con un compatibile funziona tutto, ma il nome fisso in Linux dipende dalla presa USB (lo script te lo dice). |
| B2 | Sensore a infrarossi | **MLX90614ESF-BAA** su modulo **GY-906** (con regolatore, alimentabile a 5 V) | 1 (+1 di scorta) | 10-15 € | Compralo da un venditore affidabile: in giro ci sono molti falsi. Il programma riconosce i dati rovinati. |
| B3 | Anemometro a impulsi | **Misol WH-SP-WS01** (ricambio delle stazioni Misol/Fine Offset), oppure **SparkFun Weather Meter Kit SEN-15901** | 1 | 15-25 € (Misol) / 70-90 € (SparkFun) | Entrambi: 1 impulso al secondo = 2,4 km/h (già impostato in `meteo.ini`). Lo SparkFun ha anche pluviometro e banderuola. |
| B4 | Sensore di pioggia | **Hydreon RG-11** (ottico, senza parti mobili, uscita a relè) | 1 | 80-110 € | È la voce più cara, ma è quella che protegge il telescopio. Alternativa più economica: Hydreon RG-9 (uscita elettronica, richiede un collegamento un po' diverso). **Evita** le schedine "FC-37 / YL-83": si corrodono in poche settimane. |
| B5 | Relè di appoggio per la pioggia | Relè 12 V DC con **2 contatti in scambio** + zoccolo (es. Finder 40.52.9.012.0000 + zoccolo 95.05) | 1 | 10-15 € | Divide il segnale della pioggia tra i due Arduino, tenendoli separati elettricamente. |
| B6 | Scatola per l'Arduino Nano | IP65, circa 100×100×50 mm + pressacavi | 1 | 8 € | |
| B7 | Cavo per i sensori meteo | Cavo schermato 4×0,22 mm² | 10-20 m | 10 € | |
| B8 | Pellicola di polietilene | Normale pellicola trasparente per alimenti **in polietilene (PE), non in PVC** | 1 rotolo | 2 € | Lascia passare l'infrarosso: protegge l'MLX90614 da pioggia e polvere. |
| B9 | Palo e staffe | Palo zincato Ø 30-40 mm, staffe a U | - | 20-30 € | L'anemometro va il più in alto possibile, lontano dagli ostacoli. |
| B10 | (facoltativo) Isolatore USB | Modulo con chip ADuM3160 | 1 | 15 € | Protegge il PC da disturbi e sovratensioni (fulmini vicini) che arrivano dai cavi lunghi all'aperto. |

## C. Attrezzi e minuteria

| # | Oggetto | Note |
|---|---|---|
| C1 | **Multimetro digitale** ("tester"), categoria CAT III | Es. UNI-T UT33 o simili, 20-30 €. Serve per le verifiche e il collaudo. |
| C2 | Morsetti a leva (es. Wago 221) | Per giunzioni sicure senza saldare. |
| C3 | Cavetti "Dupont" maschio-femmina | Per le prove al banco. |
| C4 | Cacciavite per morsettiere, spelafili, fascette, guaina termorestringente | |
| C5 | Spruzzino per piante | Per provare il sensore di pioggia. |
| C6 | Etichette | Per segnare cavi e prese USB. |

## Ordine consigliato degli acquisti

1. **Subito:** A1, A2, A9, B1, B2, B3, C1, C3 (servono per le prove al banco
   e per la taratura del meteo).
2. **Poi:** A4, A5, A6, A7, A8, A10, A11, B4, B5, B6-B9.
3. **Dopo i controlli della scheda** (`docs/10`): A3, ed eventualmente A12 e A14.

# 8. Taratura della stazione meteo

## Che cos'è la modalità taratura

Con `modalita = taratura` in `~/.config/osservatorio/meteo.ini` il programma:

- legge i sensori e **registra** tutto nei file CSV della cartella
  `~/osservatorio-meteo/registro/`, un file al giorno;
- calcola che cosa **avrebbe deciso** con le soglie attuali (colonna
  `sicuro_calcolato`);
- **non comunica mai "sicuro" a Ekos** e non provoca nessuna azione. In
  Ekos il meteo resta "non sicuro", con il motivo "MODALITA' TARATURA";
- ti avvisa **comunque** se un sensore si guasta.

È la modalità con cui il programma viene installato.

## Come si fa (2-3 settimane)

1. **Lascia girare il programma.** Non devi fare altro: registra una riga
   ogni 30 secondi.
2. **Tieni un quaderno.** Ogni sera e ogni mattina annota com'era il cielo,
   con data e ora:
   - sereno
   - velato
   - nuvoloso a tratti
   - coperto
   - pioggia
   - vento forte
3. **Guarda il riepilogo** quando vuoi:

   ```bash
   python3 ~/observatory/strumenti/riepilogo_taratura.py
   ```

   Esempio di risultato:

   ```
   notte del    righe  diff min  mediana    max  vento max  raffica  pioggia  nuvoloso
   2026-11-03    1320     -34.2    -31.5  -27.9        6.1     14.4       0%        0%
   2026-11-04    1320     -29.0    -12.3   -3.1       11.0     22.8       0%       71%
   2026-11-05    1320      -8.5     -5.2   -2.0       18.3     35.0      40%      100%
   ```

   Ogni colonna significa:
   - **diff** è la differenza tra temperatura del cielo e dell'aria: più è
     negativa, più il cielo è sereno;
   - **nuvoloso** è la percentuale di tempo in cui, con la soglia attuale,
     il programma avrebbe detto "nuvoloso".
4. **Confronta col quaderno.** Nelle notti che hai annotato come
   **serene**, guarda il valore "max" della differenza. Nelle notti
   **coperte** guarda il valore "min". La soglia giusta sta in mezzo.
   *Esempio:* notti serene con max −27 e notti coperte con min −9 → una
   soglia ragionevole è −18.
5. **Per un'analisi più fine** apri i file CSV con **LibreOffice Calc**
   (doppio clic). Sono già nel formato italiano, con il punto e virgola
   come separatore e la virgola decimale. Puoi fare un grafico della colonna
   "differenza" nel tempo.

## Le soglie, una per una

Tutte sono in `~/.config/osservatorio/meteo.ini`, sezione `[soglie]`.

| Soglia | Valore iniziale | Come sceglierla |
|---|---|---|
| `nuvole_soglia_differenza` | −15 | Con la taratura, come sopra. Varia con la stagione: in estate le differenze sono un po' meno negative. Ricontrollala al cambio di stagione. |
| `nuvole_rendono_non_sicuro` | si | Metti `no` se vuoi che le nuvole non facciano chiudere il tetto (verrebbero solo registrate). |
| `vento_medio_max_kmh` | 25 | Dipende dal telescopio e dalla struttura. Per molti osservatori amatoriali è un valore ragionevole. |
| `raffica_max_kmh` | 40 | Le raffiche fanno vibrare il telescopio e sollecitano il tetto. |
| pioggia | sempre attiva | Non si può disattivare. |

Nella sezione `[sicurezza]`:

| Impostazione | Valore iniziale | Significato |
|---|---|---|
| `attesa_ritorno_sicuro_min` | 15 | Dopo un allarme, quanti minuti di condizioni buone **di fila** servono per tornare "sicuro". Evita apri/chiudi continui con le nuvole sparse. |
| `dati_vecchi_dopo_s` | 30 | Se l'Arduino tace per più di 30 s → guasto, non sicuro. |
| `letture_non_valide_tollerate` | 3 | Letture sbagliate del sensore a infrarossi ammesse di fila (circa 6 secondi). |
| `aria_min/max`, `cielo_min/max` | −40/50, −80/50 | Fuori da questi limiti un valore è "assurdo". |
| `avviso_vento_zero_ore` | 48 | Se il vento è sempre zero per 48 ore, ti avvisa: l'anemometro potrebbe essere bloccato. |

**Le modifiche alle soglie hanno effetto entro 5 secondi**, senza riavviare
nulla. Se scrivi un valore sbagliato (per esempio una lettera al posto di un
numero), il programma te lo segnala con una notifica e continua a usare le
soglie precedenti.

## Fine della taratura

Quando le soglie ti convincono:

1. In `meteo.ini` cambia `modalita = taratura` in `modalita = normale` e salva.
2. Dopo `attesa_ritorno_sicuro_min` minuti di condizioni buone, Ekos mostrerà
   il meteo "OK" (verde).
3. La chiusura automatica da Ekos resta **disattivata** finché non la attivi
   tu (README, "Chiusura automatica per il meteo").

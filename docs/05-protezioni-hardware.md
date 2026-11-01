# 5. Protezioni hardware (indipendenti dal software)

Il software può avere errori, e i computer si bloccano. Per questo le
protezioni più importanti **non devono dipendere** né dal PC né
dall'Arduino. Ecco l'elenco completo, dalla più importante, con dove vanno
inserite.

| # | Protezione | Dove va | Cosa protegge | Chi la installa |
|---|---|---|---|---|
| 1 | **Pulsante di emergenza a fungo** | Sulla parete interna, vicino alla porta e raggiungibile dal telescopio | Toglie corrente alla scheda del cancello: il motore si ferma qualunque cosa facciano PC e Arduino | ⚡ Elettricista (parte a 230 V) + te (contatto per l'Arduino) |
| 2 | **Finecorsa della scheda del cancello** (magneti o molle sulla cremagliera) | Sulla cremagliera, alle due estremità della corsa | Fermano il motore a fine binario, anche se Arduino e PC sono spenti | Te, seguendo il manuale del motore |
| 3 | **Fermi meccanici di fine corsa** (tamponi in gomma, battute in acciaio) | Alle estremità del binario, un po' oltre i finecorsa | Se tutto il resto fallisce, il tetto non esce dal binario | Te / un fabbro |
| 4 | **Rilevamento ostacoli della scheda** | Nella scheda (trimmer o impostazione) | Se il tetto incontra un ostacolo (anche una persona!), il motore si ferma | Te, seguendo il manuale |
| 5 | **Tempo di lavoro massimo della scheda** | Nella scheda (impostazione) | Il motore non gira più di N secondi, indipendentemente dall'Arduino | Te, seguendo il manuale |
| 6 | **Sensore fisico di parcheggio della montatura** | Sulla montatura (contatti magnetici, vedi `03-collegamenti.md`) | Il tetto non si muove se il telescopio non è in parcheggio | Te |
| 7 | **Separazione meccanica tetto / telescopio** | Verifica di ingombro | Anche con il telescopio parcheggiato, il tetto deve passare con almeno 10 cm di margine | Te (misura) |
| 8 | **Interruttore magnetotermico-differenziale dedicato** | Quadro elettrico dell'osservatorio | Protezione delle persone e dell'impianto | ⚡ Elettricista |
| 9 | **Messa a terra** della struttura metallica, del motore e del telescopio | Impianto | Sicurezza elettrica | ⚡ Elettricista |
| 10 | **Gruppo di continuità (UPS)** | Tra presa e PC / alimentatore Arduino | PC e Arduino restano accesi durante i brevi blackout | Te (si collega a una presa) |
| 11 | **Protezione dalle sovratensioni (scaricatore)** | Quadro elettrico, ed eventualmente isolatore USB per la stazione meteo | Fulmini vicini, picchi di rete | ⚡ Elettricista |

## A proposito di "sensore di pioggia che interrompe l'alimentazione"

**Attenzione: non è una buona idea collegarlo così.** Se la pioggia togliesse
corrente al motore, il tetto **non potrebbe più chiudersi** proprio quando
serve. Peggio ancora: si fermerebbe a metà, con il telescopio esposto.

La soluzione corretta, già prevista nel progetto, è questa:

- il sensore di pioggia manda un segnale all'Arduino del tetto (oltre che al
  programma meteo);
- l'Arduino, quando il PC non risponde e la montatura è parcheggiata,
  **comanda la chiusura** (funzione `CHIUSURA_AUTONOMA_PIOGGIA`, da attivare
  dopo il collaudo).

Se vuoi una protezione contro la pioggia **del tutto indipendente**
dall'Arduino, la si può aggiungere in un secondo momento: un contatto del
sensore di pioggia collegato direttamente all'ingresso "CHIUDI" della scheda,
con un contatto di parcheggio della montatura in serie. È però
un'automazione che **non si può fermare dal PC**. Ne parliamo dopo aver visto
la scheda: dipende da come gestisce un comando "chiudi" che resta premuto.

## Dove serve l'elettricista ⚡ (riepilogo)

1. Linea dedicata all'osservatorio con interruttore magnetotermico-differenziale.
2. Collegamento della scheda del cancello alla rete a 230 V.
3. Inserimento del pulsante di emergenza sull'alimentazione della scheda,
   eventualmente con un contattore.
4. Messa a terra di struttura, motore, telescopio e quadro.
5. Eventuale lampeggiante, se a 230 V.
6. Scaricatore di sovratensione.
7. Presenza durante le misure sulla scheda accesa (`04-scheda-cancello.md`).
8. Dichiarazione di conformità dell'impianto. Dopo una modifica
   all'impianto elettrico è un obbligo di legge (D.M. 37/2008).

Tutti gli altri collegamenti sono a 5-12 V e puoi farli tu, sempre a corrente
staccata.

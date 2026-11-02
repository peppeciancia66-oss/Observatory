/*
 * =============================================================================
 *  configurazione_scheda.h  -  TUTTO cio' che dipende dalla scheda del
 *  cancello (ABM241) e dai dati non ancora verificati.
 * =============================================================================
 *
 *  Quando i controlli di docs/10-controlli-senza-corrente.md saranno fatti,
 *  si aggiornera' SOLO questo file (e le istruzioni di cablaggio in
 *  docs/03-collegamenti.md). Il resto dello sketch non cambia.
 *
 *  Ogni valore e' legato a una o piu' ipotesi "H.." elencate in
 *  docs/09-ipotesi-da-verificare.md.
 *
 *  STATO: PROVVISORIO - NESSUNA IPOTESI E' ANCORA VERIFICATA.
 * =============================================================================
 */

#ifndef CONFIGURAZIONE_SCHEDA_H
#define CONFIGURAZIONE_SCHEDA_H

// -----------------------------------------------------------------------------
// COLLEGAMENTI PREVISTI VERSO LA SCHEDA (solo promemoria, il software non li usa)
// L'Arduino NON e' mai collegato ai morsetti: tra Arduino e scheda ci sono
// sempre i contatti puliti dei rele'.
//
//   Rele' 1 APRI   -> ingresso "apri" della scheda      [H1, H2, H4, H6]
//   Rele' 2 CHIUDI -> ingresso "chiudi" della scheda    [H4, H5]
//   Rele' 3 STOP   -> ingresso "stop" della scheda      [H4, H5, H8]
//   Il morsetto comune dei tre ingressi e' da verificare [H2, H5].
//   NIENTE VA COLLEGATO finche' le ipotesi non sono verificate.
// -----------------------------------------------------------------------------

// Tipo di modulo rele' [H18]:
//   1 = modulo "attivo basso" (la maggior parte dei moduli con optoisolatori)
//   0 = modulo "attivo alto" (es. Arduino 4 Relays Shield)
#define RELE_ATTIVO_BASSO  1

// Durata della "pressione del tasto" sugli ingressi della scheda [H15].
// Ipotesi: mezzo secondo basta perche' la scheda riconosca il comando.
#define IMPULSO_MS             500
// Pausa dopo ogni impulso, prima di un altro comando.
#define PAUSA_DOPO_IMPULSO_MS  300

// Se 1, quando il tetto raggiunge il NOSTRO finecorsa l'Arduino preme anche
// STOP, in aggiunta ai finecorsa interni del motore [H9, H14].
// Ipotesi: premere STOP a motore gia' fermo non fa nulla. Se il collaudo
// dimostra il contrario, mettere 0.
#define FERMA_AL_FINECORSA  1

// Se 1, il comando ABORT di Ekos preme STOP anche quando l'Arduino crede che
// il tetto sia fermo (utile se il tetto e' stato mosso col telecomando) [H9].
#define STOP_ANCHE_DA_FERMO  1

// Tempo massimo di un movimento completo, in secondi [H17].
// Calcolo provvisorio dalla targhetta: velocita' 12 m/min = 0,2 m/s;
// cremagliera da 6 m -> al massimo 30 s di corsa; + 30% di margine e il
// tempo dell'avvio lento -> 45 s.
// AL COLLAUDO (fase C.3): misura la corsa vera col cronometro e scrivi qui
// circa 1,3 volte il tempo piu' lungo tra apertura e chiusura.
#define TEMPO_MAX_MOVIMENTO_S  45

// Entro quanti secondi il tetto deve lasciare il finecorsa di partenza
// (o, con il sensore di movimento, iniziare a muoversi) [H1, H15].
// Tiene conto dell'avvio lento (DIP 1) [H11].
#define TEMPO_MAX_DISTACCO_S   8

// Sensore di movimento (facoltativo, consigliato): un contatto reed fisso e
// dei magneti sul tetto a distanza regolare. Ogni magnete che passa = un
// impulso. Serve a sapere se il tetto si muove DAVVERO.
#define USA_SENSORE_MOVIMENTO  0
// Il tetto e' "in movimento" se l'ultimo impulso e' piu' recente di cosi'.
// Con magneti ogni 30 cm e 0,2 m/s passa un magnete ogni 1,5 s: 4000 ms
// lasciano margine. Da ricalcolare con la distanza vera dei magneti.
#define FINESTRA_MOVIMENTO_MS  4000

// Dopo la fine di un movimento comandato, per quanti millisecondi ignorare
// i movimenti "non comandati" (inerzia del tetto, rimbalzi dei finecorsa).
#define TOLLERANZA_DOPO_MOVIMENTO_MS  5000

#endif

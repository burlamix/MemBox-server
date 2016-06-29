#include <ops.h>
#include <message.h>
#include <assert.h>
#include <icl_hash.h>

/**
 * @function sendReply
 * @brief invia un messaggio di risposta al client
 *
 * @param fd file descriptor nel quale scrivere il messaggio
 * @param hdr puntatore alla struttura message_hdr_t che rappresenta il messaggio di risposta
 *
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int sendReply(long fd, message_hdr_t *hdr);

/**
 * @function SizeofRep
 * @brief calcola la dimensione della repository
 *
 * @param repo repository della quale si deve calcolare la dimensione
 *
 * @returns la dimensione in caso di successo,-1 in caso di fallimento
 */
int gest_op(message_t* mex,long fd, icl_hash_t* repository,struct statistics  *mboxStats, pthread_mutex_t lk_stat );

/**
 * @function put_op
 * @brief funzione per la gestione dell'inserimento di un oggetto nel repository
 *
 * @param new_data   puntatore all'oggetto da inserire
 * @param repository   puntatore alla tabella hash nella quale inserire l'oggetto
 * @param fd   file descriptor per comunicazione dell'esito al client richiedente
 * @param mboxStats puntatore alla struttura per l'aggiornamento delle statistiche
 * @param lk_stat variabile di mutua esclusione per l'accesso alla struttura delle statistiche
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int put_op(message_t* mex, icl_hash_t* repository,long fd,struct statistics  *mboxStats, pthread_mutex_t lk_stat );

/**
 * @function update_op
 * @brief funzione per la gestione dell'aggiornamento di un oggetto nel repository
 *
 * @param new_mex   puntatore al messaggio di richiesta aggiornamento
 * @param repository   puntatore alla tabella hash nella quale cercare l'oggetto da aggiornare
 * @param fd   file descriptor per comunicazione dell'esito al client richiedente
 * @param mboxStats puntatore alla struttura per l'aggiornamento delle statistiche
 * @param lk_stat variabile di mutua esclusione per l'accesso alla struttura delle statistiche
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int update_op(message_t* mex ,icl_hash_t* repository, long fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat );

/**
 * @function remove_op
 * @brief funzione per la gestione della rimozione di un oggetto nel repository
 *
 * @param repository   puntatore alla tabella hash nella quale cercare l'oggetto da eliminare
 * @param key   chiave identificativa dell'oggetto da eliminare
 * @param fd   file descriptor per comunicazione dell'esito al client richiedente
 * @param mboxStats puntatore alla struttura per l'aggiornamento delle statistiche
 * @param lk_stat variabile di mutua esclusione per l'accesso alla struttura delle statistiche
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int remove_op(icl_hash_t* repository, membox_key_t key,long fd,struct statistics  *mboxStats, pthread_mutex_t lk_stat );

/**
 * @function get_op
 * @brief funzione per la gestione della ricerca di un oggetto nel repository
 *
 * @param repository   puntatore alla tabella hash nella quale cercare l'oggetto
 * @param key   chiave identificativa dell'oggetto
 * @param fd   file descriptor per comunicazione dell'esito al client richiedente
 * @param mboxStats puntatore alla struttura per l'aggiornamento delle statistiche
 * @param lk_stat variabile di mutua esclusione per l'accesso alla struttura delle statistiche
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int get_op(icl_hash_t* repository, membox_key_t key,long fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat );

/**
 * @function lock_op
 * @brief funzione per accedere in mutua esclusione repository
 *
 * @param repository   puntatore alla tabella hash che implementa la repository
 * @param mboxStats puntatore alla struttura per l'aggiornamento delle statistiche
 * @param lk_stat variabile di mutua esclusione per l'accesso alla struttura delle statistiche
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int lock_op(long fd,icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat );

/**
 * @function unlock_op
 * @brief funzione per rilasciare la mutua esclusione della repository
 *
 * @param fd filedescriptor della comunicazione nella quale è chiesta l'unlock
 * @param repository   puntatore alla tabella hash che implementa la repository
 * @param mboxStats puntatore alla struttura per l'aggiornamento delle statistiche
 * @param lk_stat variabile di mutua esclusione per l'accesso alla struttura delle statistiche
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int unlock_op(long fd, icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat );

/**
 * @function gest_op
 * @brief funzione per la gestione delle operazioni sulla repository
 *
 * @param mex puntatore al messaggio di richiesta dell'operazione
 * @param fd filedescriptor della comunicazione nella quale è chiesta l'operazione
 * @param repository   puntatore alla tabella hash che implementa la repository
 * @param mboxStats puntatore alla struttura per l'aggiornamento delle statistiche
 * @param lk_stat variabile di mutua esclusione per l'accesso alla struttura delle statistiche
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int gest_op(message_t * mex,long fd, icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat);
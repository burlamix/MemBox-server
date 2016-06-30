#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ops.h>
#include <message.h>
#include <assert.h>
#include <icl_hash.h>
#include <err_man.h>
#include <stats.h>
#include <connections.h>

int StorageSize = 3000;
int StorageByteSize = 7000;
int MaxObjSize = 100;


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
int sendReply(long fd, message_hdr_t *hdr) {

  ec_meno1_c(write(fd, hdr , sizeof(message_hdr_t)), "impossibile inviare risposta", return -1);
  return 0;
}



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
int put_op(message_t* new_data,  icl_hash_t* repository,  int fd, struct statistics *mboxStats, pthread_mutex_t lk_stat ) {

  message_hdr_t risp;
  memset(&risp, 0, sizeof(message_hdr_t));
  message_data_t *obj;
  int op;

  //viene aggiornato il numero di put e alcuni valori delle statistiche vengono salvati in delle variabili locali
  Pthread_mutex_lock(&lk_stat);
  mboxStats->nput++;
  int newdim = mboxStats->current_size + (new_data->data.len);
  int curr_obj = mboxStats->current_objects;
  Pthread_mutex_unlock(&lk_stat);

  //si verifica che la repository non abbia raggiunto il massimo numero di elementi
  if ( repository->StorageSize != 0 && curr_obj >= repository->StorageSize) {
    risp.op = OP_PUT_TOOMANY;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nput_failed++;
    Pthread_mutex_unlock(&lk_stat);
    ec_meno1_np(sendReply( fd, &risp), return -1);
    return 0;
  }
  //la condizione sulla dimensione della repository viene controllata prima di fare l'inserimento
  if (repository->StorageByteSize != 0 && newdim > repository->StorageByteSize) {
    risp.op = OP_PUT_REPOSIZE;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nput_failed++;
    Pthread_mutex_unlock(&lk_stat);
    ec_meno1_np(sendReply( fd, &risp), return -1);
    return 0;
  }
  //si controlla la dimensione massima dell'oggetto
  if (repository->MaxObjSize != 0 && new_data->data.len > repository->MaxObjSize) {
    risp.op = OP_PUT_SIZE;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nput_failed++;
    Pthread_mutex_unlock(&lk_stat);
    ec_meno1_np(sendReply( fd, &risp), return -1);
    return 0;
  }
  //creazione dell'oggetto da inserire nel repository
  ec_null_c(obj = calloc(1, sizeof(message_data_t)), "impossibile allocare oggetto", op = -1);
  if (obj != NULL) {
    obj->len = new_data->data.len;
    obj->buf = (char*)malloc(obj->len);
    memcpy(obj->buf, new_data->data.buf, new_data->data.len);
    op = icl_hash_insert( repository, new_data->hdr.key, obj);
  }
  //in base all'esito dell'inserimento vengono aggiornate le statistiche e
  //settato il messaggio di risposta
  switch (op) {
  case 0 :
    risp.op = OP_OK;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->current_size = newdim;
    if (newdim > mboxStats->max_size)
      mboxStats->max_size = newdim;
    mboxStats->current_objects++;
    if (mboxStats->current_objects > mboxStats->max_objects)
      mboxStats->max_objects++;
    Pthread_mutex_unlock(&lk_stat);
    break;
  case -1 :
  case -3 :
    risp.op = OP_FAIL;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nput_failed++;
    Pthread_mutex_unlock(&lk_stat);
    break;
  case -2 :
    risp.op = OP_PUT_ALREADY;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nput_failed++;
    Pthread_mutex_unlock(&lk_stat);
    break;
  }
  ec_meno1_np(sendReply( fd, &risp), return -1);
  return 0;
}


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
 int update_op(message_t* new_mex, icl_hash_t* repository, int fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat) {

  //viene cercato l'oggetto da aggiornare nella tabella hash
  message_data_t* dato = (message_data_t*) icl_hash_find( repository, new_mex->hdr.key);
  message_hdr_t risp;
  memset(&risp, 0, sizeof(message_hdr_t));
  //caso in cui l'oggetto non è presente
  if (dato == NULL) {
    risp.op = OP_UPDATE_NONE;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nupdate++;
    mboxStats->nupdate_failed++;
    Pthread_mutex_unlock(&lk_stat);
    ec_meno1_np(sendReply( fd, &risp), return -1);
    return 0;
  }
  //caso in cui non si ha corrispondenza tra le dimensioni
  if (dato->len != new_mex->data.len) {
    risp.op = OP_UPDATE_SIZE;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nupdate++;
    mboxStats->nupdate_failed++;
    Pthread_mutex_unlock(&lk_stat);
  } else {
    //aggiornamento
    memcpy((void*)dato->buf, (void*)new_mex->data.buf, dato->len); // qui non va fatto così ma senno
    risp.op = OP_OK;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nupdate++;
    Pthread_mutex_unlock(&lk_stat);
  }
  //invio della risposta al client
  ec_meno1_np(sendReply( fd, &risp), return -1);

  return 0;
}


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
int remove_op(icl_hash_t* repository, membox_key_t key, int fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat) {

  //viene rimosso l'oggetto dalla repository
  int op = icl_hash_delete( repository, key);
  message_hdr_t risp;
  memset(&risp, 0, sizeof(message_hdr_t));
  //controllo esito eliminazione e aggiornamento statistiche
  if (op >= 0) {
    //caso in cui oggetto è presete e eliminazione va a buon fine
    risp.op = OP_OK;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nremove++;
    mboxStats->current_size -= op;
    mboxStats->current_objects--;
    Pthread_mutex_unlock(&lk_stat);
  } else {
    //caso in cui oggetto da rimuore non presente
    risp.op = OP_REMOVE_NONE;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nremove++;
    mboxStats->nremove_failed++;
    Pthread_mutex_unlock(&lk_stat);
  }
  //invio risposta al client
  ec_meno1_np(sendReply( fd, &risp), return -1);
  return 0;
}

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
int get_op(icl_hash_t* repository, membox_key_t key, int fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat  ) {

  message_data_t* dato;
  //viene cercato l'oggetto con chiave richiesta
  dato = (message_data_t*) icl_hash_find( repository, key);

  message_hdr_t risp;
  memset(&risp, 0, sizeof(message_hdr_t));

  risp.key = key;
  //gestione dell'esito dell'operazione
  if (dato == NULL) { //caso in cui l'oggetto non è presente
    risp.op = OP_GET_NONE;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nget++;
    mboxStats->nget_failed++;
    Pthread_mutex_unlock(&lk_stat);
  } else {
    risp.op = OP_OK;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nget++;
    Pthread_mutex_unlock(&lk_stat);

  }
  //invio della risposta di esito dell'operazione al client
  ec_meno1_np(sendReply( fd, &risp), return -1);
  //in caso di esito positivo invio del dato
  if (risp.op == OP_OK) {
    ec_meno1_np(write(fd, &(dato->len ), sizeof(int)), return -1);
    char * aus;
    aus = dato->buf ;
    int scritti = 0;
    int da_scrivere = dato->len;
    while (da_scrivere > 0) {
      scritti = write(fd, aus , da_scrivere );
      aus = aus + scritti - 1;
      da_scrivere = da_scrivere - scritti;
    }
  }
  return 0;
}

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
int lock_op(int fd, icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat ) {
  message_hdr_t risp;
  memset(&risp, 0, sizeof(message_hdr_t));

  Pthread_mutex_lock(& repository->lk_repo);
  //ulteriore controllo che in realà non è necessario in quanto se la lock è presa la gestione avviene in gest_op
  if (repository->repo_l) { //caso in cui la lock è già presa
    Pthread_mutex_unlock(& repository->lk_repo);

    risp.op = OP_LOCKED;
    ec_meno1_np(sendReply( fd, &risp), return -1);
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nlock++;
    mboxStats->nlock_failed++;
    Pthread_mutex_unlock(&lk_stat);

  } else {
    //viene bloccato il repository settando repo_l e fd in modo che nuove richieste di operazioni non siano accettate
    repository->repo_l = 1;
    repository->fd = fd;
    pthread_mutex_unlock(& repository->lk_repo);

    pthread_mutex_lock(&(repository->lk_job_c));
    //si attende che terminino le operazioni sulla repository
    while (repository->job_c > 1) {
      printf("\nvalore di job =%d\n", repository->job_c);

      pthread_cond_wait(&(repository->cond_job), &(repository->lk_job_c));
    }
    printf("\nvalore di job =%d\n", repository->job_c);

    pthread_mutex_unlock(& repository->lk_job_c);
    //il repository è stato bloccato, viene mandata risposta al client e aggiornate le statistiche
    risp.op = OP_OK;
    ec_meno1_np(sendReply( fd, &risp), return -1);
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nlock++;
    Pthread_mutex_unlock(&lk_stat);
  }
  return 0;
}

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
int unlock_op(int fd, icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat ) {

  message_hdr_t risp;
  memset(&risp, 0, sizeof(message_hdr_t));


  Pthread_mutex_lock(& repository->lk_repo);
  if (repository->repo_l && fd == repository->fd) { //caso in cui la lock è stata presa in maniera corretta
    repository->repo_l = 0;
    Pthread_mutex_unlock(&(repository->lk_repo));

    risp.op = OP_OK;
    ec_meno1_np(sendReply( fd, &risp), return -1);
  } else { //caso in cui la lock non è già stata presa o è stata presa da qualcunaltro
    Pthread_mutex_unlock(& repository->lk_repo);

    risp.op = OP_LOCK_NONE;
    ec_meno1_np(sendReply( fd, &risp), return -1);
  }
  return 0;
}


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
int gest_op(message_t * mex, long fd, icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat) {

  int ris_op = 0;
  if (mex->hdr.op == PUT_OP || mex->hdr.op == UPDATE_OP ) {
    readData(fd, &mex->data);
  } else {
    printf("********************read non legge! \n" );
  }


  //controlla che la repository non sia bloccata da una operazione LOCK
  Pthread_mutex_lock(&(repository->lk_repo));
  if (repository->repo_l == 0 || repository->fd == fd) {

    //se la repository non è bloccata viene incrementato il valore che identifica il numero di operazioni in esecuzione sulla repository
    Pthread_mutex_lock(&(repository->lk_job_c));
    repository->job_c++;
    Pthread_mutex_unlock(&repository->lk_job_c);
    Pthread_mutex_unlock(&(repository->lk_repo));

    //viene eseguita l'operazione richiesta
    switch (mex->hdr.op) {

    case PUT_OP:
      ris_op = put_op(mex, repository, fd, mboxStats, lk_stat );
      break;

    case UPDATE_OP:
      ris_op =  update_op(mex, repository, fd, mboxStats, lk_stat );
      break;

    case REMOVE_OP:
      ris_op = remove_op(repository, mex->hdr.key, fd, mboxStats, lk_stat );
      break;

    case GET_OP:
      ris_op = get_op( repository, mex->hdr.key, fd, mboxStats, lk_stat );
      break;

    case LOCK_OP:
      ris_op = lock_op(fd, repository, mboxStats, lk_stat );
      break;

    case UNLOCK_OP:
      ris_op = unlock_op(fd, repository, mboxStats, lk_stat );
      break;
    default:
      fprintf(stderr, "Invalid request\n");
      return -1;
    }

    printf(" risultato dell op=%d\n", ris_op);
    //posso liberare il buffer che verrà riallocato nuovamente alla prossima readData
    if (mex->hdr.op == PUT_OP || mex->hdr.op == UPDATE_OP ) {
      free(mex->data.buf);
    }


    Pthread_mutex_lock(&(repository->lk_job_c));
    repository->job_c--;

    //nel caso in cui non ci siano più lavori in esecuzione allora viene attivata la lock
    if (repository->job_c == 0)
      Pthread_cond_signal(&(repository->cond_job));//nel caso nessuno abbia chiesto la lock la signal andrà persa
    Pthread_mutex_unlock(&(repository->lk_job_c));
  } else {
    //aggiorno le statistiche con i fallimenti
    Pthread_mutex_lock(&lk_stat);
    switch (mex->hdr.op) {

    case PUT_OP:
      mboxStats->nput_failed++;
      break;

    case UPDATE_OP:
      mboxStats->nupdate_failed++;
      break;

    case REMOVE_OP:
      mboxStats->nremove_failed++;
      break;

    case GET_OP:
      mboxStats->nget_failed++;
      break;

    case LOCK_OP:
      mboxStats->nlock_failed++;
      break;
    default:
      fprintf(stderr, "Invalid request\n");
      return -1;
    }
    Pthread_mutex_unlock(&lk_stat);
    //risposta al client di operazione non concessa perchè il repository è bloccato
    Pthread_mutex_unlock(&(repository->lk_repo));
    message_hdr_t *risp = malloc(sizeof(message_hdr_t));
    risp->op = OP_LOCKED;
    ec_meno1_np(sendReply( fd, risp), return -1);
    free(risp);

  }

  return ris_op;
}

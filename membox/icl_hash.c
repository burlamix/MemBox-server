/**
 * @file icl_hash.c
 *
 * Implementazione di una tabella hash per la realizzazione di un repository 
 * di message_data_t come definito nel @file message.h
 *
 * Il codice per l'implementaione è stato ottenuto con delle modifiche al codice 
 ] di Jakub kurzak che si riferiva a delle tabelle hash generiche.
 * @author Jakub Kurzak, Simone Spagnoli, Eleonora Di Gregorio
 */
/* $Id: icl_hash.c 2838 2011-11-22 04:25:02Z mfaverge $ */
/* $UTK_Copyright: $ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <err_man.h>




#include "icl_hash.h"

#include <limits.h>


#define BITS_IN_int     ( sizeof(int) * CHAR_BIT )
#define THREE_QUARTERS  ((int) ((BITS_IN_int * 3) / 4))
#define ONE_EIGHTH      ((int) (BITS_IN_int / 8))
#define HIGH_BITS       ( ~((unsigned int)(~0) >> ONE_EIGHTH ))
/**
 * A simple string hash.
 *
 * An adaptation of Peter Weinberger's (PJW) generic hashing
 * algorithm based on Allen Holub's version. Accepts a pointer
 * to a datum to be hashed and returns an unsigned integer.
 * From: Keith Seymour's proxy library code
 *
 * @param[in] key -- the string to be hashed
 *
 * @returns the hash index
 */


static unsigned int
hash_pjw(void* key)
{
    char *datum = (char *)key;
    unsigned int hash_value, i;

    if(!datum) return 0;

    for (hash_value = 0; *datum; ++datum) {
        hash_value = (hash_value << ONE_EIGHTH) + *datum;
        if ((i = hash_value & HIGH_BITS) != 0)
            hash_value = (hash_value ^ (i >> THREE_QUARTERS)) & ~HIGH_BITS;
    }
    return (hash_value);
}


void freedata(message_data_t* aus) {
  free(aus->buf);
}


/**
 * @function icl_hash_create
 * @brief crea una nuova tabella hash
 *
 * @param nbuckets numero di buckets da creare
 * @param ss massima dimensione che può assumere il repository
 * @param sbs massima dimensione che può assumere il repository espressa in byte
 * @param mos massima dimensione che può assumere un oggetto del repository
 *
 * @returns il puntatore alla tabella hash in caso di successo, NULL in caso di fallimento
 */

icl_hash_t *
icl_hash_create( int nbuckets, int ss, int sbs, int mos){
    icl_hash_t *ht;
    int i,aus;

    ht= (icl_hash_t*)malloc(sizeof(icl_hash_t));
    if(!ht) return NULL;
    //inizializzo le informazioni della tabella hash
    ht->nbuckets = nbuckets;
    ht->StorageSize=ss;
    ht->StorageByteSize=sbs;
    ht->MaxObjSize=mos;

    if((aus=pthread_mutex_init(&ht->lk_repo, NULL))!=0) { free(ht); return NULL;}
    ht->repo_l=0;
    ht->fd=-1;
    
    if((aus=pthread_mutex_init(&ht->lk_job_c, NULL))!=0) {free(ht); return NULL; }
    ht->job_c=0;
    if((aus=pthread_cond_init(&ht->cond_job,NULL))!=0) { free(ht); return NULL; }
    
    //alloco la lista delle liste di trabocco
    ht->buckets = (icl_entry_t**)malloc(nbuckets * sizeof(icl_entry_t*));
    if(!ht->buckets) {free(ht); return NULL;}
    //alloco la lista delle variabili di mutua esclusione e di condizione
    ht->lkline = (icl_entry_lk*)malloc(nbuckets * sizeof(icl_entry_lk));
    if(!ht->lkline) {free(ht); free(ht->buckets); return NULL; }

    //inizializzo variabili di condizione e mutua esclusione
    for(i=0;i<ht->nbuckets;i++){
        ht->buckets[i] = NULL;
        if((aus=pthread_cond_init(&(ht->lkline[i].cond_line),NULL))!=0) {free(ht); free(ht->buckets); free(ht->lkline);return NULL;}
        if((aus=pthread_mutex_init(&(ht->lkline[i].mutex_line),NULL))!=0) {free(ht); free(ht->buckets); free(ht->lkline);return NULL;}
        ht->lkline[i].c=-1;
    }
    return ht;
        
}



/**
 * @function icl_hash_find
 * @brief cerca un elemento nella tabella hash
 *
 * @param ht  la tabella hash nella quale cercare l'elemento
 * @param key   la chiave dell'elemento che deve essere cercato
 *
 * @returns il puntatore all'elemento che deve essere cercato se è presente,
 *          NULL se l'elemento non è trovato
 */

message_data_t *
icl_hash_find(icl_hash_t *ht, unsigned long key){
    icl_entry_t* curr;
    message_data_t* aus;
    unsigned int hash_val;

    if(!ht ) return NULL;
    //calcolo il valore della chiave
    hash_val= hash_pjw((void*)&key)% ht->nbuckets;
    //acquisisco la mutua esclusione sulla lista di trabocco
    Pthread_mutex_lock(&ht->lkline[hash_val].mutex_line);
    while(ht->lkline[hash_val].c!=-1){
            Pthread_cond_wait(&ht->lkline[hash_val].cond_line,&ht->lkline[hash_val].mutex_line);
    }
    ht->lkline[hash_val].c=1;
    Pthread_mutex_unlock(&ht->lkline[hash_val].mutex_line);
    //scorro lista di trabocco
    aus=NULL;
    for (curr=ht->buckets[hash_val]; curr != NULL; curr=curr->next){
        if ( curr->key==key){
            aus=curr->data;
            break;
        }
    }
    //permetto di fare operazione sulla riga a chi è in attesa
    Pthread_mutex_lock(&ht->lkline[hash_val].mutex_line);
    ht->lkline[hash_val].c=-1;
    Pthread_cond_signal(&ht->lkline[hash_val].cond_line);
    Pthread_mutex_unlock(&ht->lkline[hash_val].mutex_line);

    return aus;

}

/**
 * @function icl_hash_insert
 * @brief inserisce un elemento nella tabella hash
 *
 * @param ht   la tabella hash nel quale bisogna inserire l'elemento
 * @param key   la chiave del nuovo elemento
 * @param data  l'elemento da inserire nella tabella hash
 *
 * @returns 0 in caso di successo, -1 per il fallimento dell'allocazione, 
 *          -2 se l'elemento esite già, -3 se i parametri passati non sono corretti
 */

int
icl_hash_insert(icl_hash_t *ht, unsigned long key, message_data_t* data)
{
    icl_entry_t *curr;
    unsigned int hash_val;
    int aus=0;

    if(!ht) return -3;

   //calcolo la chiave
    hash_val=hash_pjw((void*)&key)% ht->nbuckets;

    printf("prima attesa insert\n");
            fflush(stdout);
    //acquisisco la mutua esclusione sulla lista di trabocco
    Pthread_mutex_lock(&(ht->lkline[hash_val].mutex_line));
    
    while(ht->lkline[hash_val].c!=-1){
            Pthread_cond_wait(&(ht->lkline[hash_val].cond_line),&(ht->lkline[hash_val].mutex_line));
    }
    ht->lkline[hash_val].c=1;
    Pthread_mutex_unlock(&(ht->lkline[hash_val].mutex_line));

    for (curr=ht->buckets[hash_val]; curr != NULL; curr=curr->next){
        if ( curr->key==key)
            aus=-2; /* key already exists */
    }

    /* if key was not found */
    if(aus!=-2){
        curr = (icl_entry_t*)malloc(sizeof(icl_entry_t));
        if(!curr) aus=-1;
            if(aus!=-1){
                curr->key = key;
                curr->data = data;
                curr->next = ht->buckets[hash_val]; /* add at start */
                ht->buckets[hash_val] = curr;
            }
    }
    //permetto di fare operazione sulla riga a chi è in attesa
    Pthread_mutex_lock(&(ht->lkline[hash_val].mutex_line));
    ht->lkline[hash_val].c=-1;
    Pthread_cond_signal(&(ht->lkline[hash_val].cond_line));
    Pthread_mutex_unlock(&(ht->lkline[hash_val].mutex_line));

    return aus;

}


/**
 * @function icl_hash_delete
 * @brief elimina un elemento dalla tabella hash
 *
 * @param ht tabella hash dalla quale eliminare l'elemento
 * @param key chiave dell'elemento che deve essere eliminato
 *
 * @returns la dimensione dell'elemento in caso di successo, -1 in caso di fallimento.
 */

int icl_hash_delete(icl_hash_t *ht, unsigned long key ){
    icl_entry_t *curr, *prev;
    unsigned int hash_val;
    int aus;
    if(!ht ) return -1;

    hash_val=hash_pjw((void*)&key)% ht->nbuckets;

    Pthread_mutex_lock(&(ht->lkline[hash_val].mutex_line));
     while(ht->lkline[hash_val].c!=-1){
            Pthread_cond_wait(&(ht->lkline[hash_val].cond_line),&(ht->lkline[hash_val].mutex_line));
    }
    ht->lkline[hash_val].c=1;
    Pthread_mutex_unlock(&(ht->lkline[hash_val].mutex_line));

    aus=-1;
    prev= NULL;
    for(curr=ht->buckets[hash_val]; curr !=NULL;curr=curr->next){
        if(curr->key==key){
            if(prev==NULL){
                ht->buckets[hash_val] =curr->next;
            }else{
                prev->next=curr->next;
            }
            aus=curr->data->len;
            free(curr->data->buf);
            free(curr->data);
            free(curr);
            break;

        }
        prev= curr;
    }

    Pthread_mutex_lock(&(ht->lkline[hash_val].mutex_line));
    ht->lkline[hash_val].c=-1;
    Pthread_cond_signal(&(ht->lkline[hash_val].cond_line));
    Pthread_mutex_unlock(&(ht->lkline[hash_val].mutex_line));


    return aus;
}




/**
 * @function icl_hash_destroy 
 * @brief elimina tutti gli elementi della tabella hash e la tabella stessa
 *
 * @param ht   la tabella che deve essere libarata
 *
 */
void
icl_hash_destroy(icl_hash_t *ht)
{
    icl_entry_t *bucket, *curr, *next;
    int i;

    for (i=0; i<ht->nbuckets; i++) {
        bucket = ht->buckets[i];
        for (curr=bucket; curr!=NULL; ) {
            next=curr->next;
            free(curr->data->buf);
            free(curr->data);
            free(curr);
            curr=next;
        }
    }

    if(ht->buckets) free(ht->buckets);
    if(ht->lkline) free(ht->lkline);
    if(ht) free(ht);

}

/**
 * @function icl_hash_dump
 * @brief stampa il contenuto della tabella hash su un file
 *
 * @param stream il file aperto sul quale viene stampato il contenuto della tabella hash
 * @param ht la tabella hash da stampare
 *
 * @returns 0 in caso di successo, -1 in caso di errore
 */

int
icl_hash_dump(FILE* stream, icl_hash_t* ht)
{
    icl_entry_t *bucket, *curr;
    int i;

    if(!ht) return -1;

    for(i=0; i<ht->nbuckets; i++) {
        bucket = ht->buckets[i];
        for(curr=bucket; curr!=NULL;curr=curr->next ) {
            if(curr->key>=0)
                fprintf(stream, "icl_hash_dump: %ld: %d %s\n", curr->key, curr->data->len, (char*)curr->data->buf);
                fflush(stream);
        }
    }

    return 0;
}


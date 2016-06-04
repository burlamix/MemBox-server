/**
 * @file icl_hash.c
 *
 * Dependency free hash table implementation.
 *
 * This simple hash table implementation should be easy to drop into
 * any other peice of code, it does not depend on anything else :-)
 * 
 * @author Jakub Kurzak
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
 * Create a new hash table.
 *
 * @param[in] nbuckets -- number of buckets to create
 * @param[in] hash_function -- pointer to the hashing function to be used
 * @param[in] hash_key_compare -- pointer to the hash key comparison function to be used
 *
 * @returns pointer to new hash table.
 */

icl_hash_t *
icl_hash_create( int nbuckets, int ss, int sbs, int mos){
    icl_hash_t *ht;
    int i;

    ht= (icl_hash_t*)malloc(sizeof(icl_hash_t));
    if(!ht) return NULL;

    ht->nbuckets = nbuckets;
    ht->nentries = 0;
    ht->StorageSize=ss;
    ht->StorageByteSize=sbs;
    ht->MaxObjSize=mos;
    pthread_mutex_init(&ht->lk_repo, NULL);
    ht->repo_l=0;
    ht->fd=-1;
    //gestione errore
    
    pthread_mutex_init(&ht->lk_job_c, NULL);
    ht->job_c=0;
    //gestione errore
    pthread_cond_init(&ht->cond_job,NULL);
    
    
    ht->buckets = (icl_entry_t**)malloc(nbuckets * sizeof(icl_entry_t*));
    if(!ht->buckets) return NULL;
    ht->lkline = (icl_entry_lk*)malloc(nbuckets * sizeof(icl_entry_lk));
    if(!ht->lkline) return NULL;

    for(i=0;i<ht->nbuckets;i++){
        ht->buckets[i] = NULL;
        pthread_cond_init(&(ht->lkline[i].cond_line),NULL);
        pthread_mutex_init(&(ht->lkline[i].mutex_line),NULL);
        ht->lkline[i].c=-1;
    }
    return ht;
        
}



/**
 * Search for an entry in a hash table.
 *
 * @param ht -- the hash table to be searched
 * @param key -- the key of the item to search for
 *
 * @returns pointer to the data corresponding to the key.
 *   If the key was not found, returns NULL.
 */

message_data_t *
icl_hash_find(icl_hash_t *ht, unsigned long key){
    icl_entry_t* curr;
    message_data_t* aus;
    unsigned int hash_val;

    if(!ht ) return NULL;
   
    hash_val= hash_pjw((void*)&key)% ht->nbuckets;

    Pthread_mutex_lock(&ht->lkline[hash_val].mutex_line);
    while(ht->lkline[hash_val].c!=-1){
            Pthread_cond_wait(&ht->lkline[hash_val].cond_line,&ht->lkline[hash_val].mutex_line);
    }
    ht->lkline[hash_val].c=1;
    Pthread_mutex_unlock(&ht->lkline[hash_val].mutex_line);

    aus=NULL;
    for (curr=ht->buckets[hash_val]; curr != NULL; curr=curr->next)
        if ( curr->key==key)
            aus=curr->data;

    Pthread_mutex_lock(&ht->lkline[hash_val].mutex_line);
    ht->lkline[hash_val].c=-1;
    Pthread_cond_signal(&ht->lkline[hash_val].cond_line);
    Pthread_mutex_unlock(&ht->lkline[hash_val].mutex_line);

    return aus;

}

/**
 * Insert an item into the hash table.
 *
 * @param ht -- the hash table
 * @param key -- the key of the new item
 * @param data -- pointer to the new item's data
 *
 * @returns 0 if is ok, -1 on a malloc error, -2 if the object altready exists., -3 invalid argument
 */

int
icl_hash_insert(icl_hash_t *ht, unsigned long key, message_data_t* data)
{
    icl_entry_t *curr;
    unsigned int hash_val;
    int aus=0;

    if(!ht) return -3;

   
    hash_val=hash_pjw((void*)&key)% ht->nbuckets;

    printf("prima attesa insert\n");
            fflush(stdout);

    Pthread_mutex_lock(&(ht->lkline[hash_val].mutex_line));
    
    while(ht->lkline[hash_val].c!=-1){
            pthread_cond_wait(&(ht->lkline[hash_val].cond_line),&(ht->lkline[hash_val].mutex_line));
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
                ht->nentries++;
            }
    }
    Pthread_mutex_lock(&(ht->lkline[hash_val].mutex_line));
    ht->lkline[hash_val].c=-1;
    Pthread_cond_signal(&(ht->lkline[hash_val].cond_line));
    Pthread_mutex_unlock(&(ht->lkline[hash_val].mutex_line));

    return aus;

}


/**
 * Free one hash table entry located by key (key and data are freed using functions).
 *
 * @param ht -- the hash table to be freed
 * @param key -- the key of the new item
 * @param free_key -- pointer to function that frees the key
 * @param free_data -- pointer to function that frees the data
 *
 * @returns len of data on success, -1 on failure.
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
            ht->nentries--; //ci sarà da mettere in mutua escusione??
            aus=curr->data->len;
            freedata(curr->data);
            free(curr);

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
 * Free hash table structures (key and data are freed using functions).
 *
 * @param ht -- the hash table to be freed
 * @param free_key -- pointer to function that frees the key
 * @param free_data -- pointer to function that frees the data
 *
 * @returns 0 on success, -1 on failure.
 */
int
icl_hash_destroy(icl_hash_t *ht)
{
    icl_entry_t *bucket, *curr, *next;
    int i;

    if(!ht) return -1;

    for (i=0; i<ht->nbuckets; i++) {
        bucket = ht->buckets[i];
        for (curr=bucket; curr!=NULL; ) {
            next=curr->next;
            freedata(curr->data);
            free(curr);
            curr=next;
        }
    }

    if(ht->buckets) free(ht->buckets);
    if(ht->lkline) free(ht->lkline);
    if(ht) free(ht);

    return 0;
}

/**
 * Dump the hash table's contents to the given file pointer.
 *
 * @param stream -- the file to which the hash table should be dumped
 * @param ht -- the hash table to be dumped
 *
 * @returns 0 on success, -1 on failure.
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


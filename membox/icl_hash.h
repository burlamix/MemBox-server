/**
 * @file
 *
 * Header file for icl_hash routines.
 *
 */
/* $Id$ */
/* $UTK_Copyright: $ */

#ifndef icl_hash_h
#define icl_hash_h

#include <stdio.h>
#include <pthread.h>
#include <message.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    pthread_cond_t cond_line;
    pthread_mutex_t mutex_line;
} icl_entry_lk;

typedef struct icl_entry_s {
    unsigned long key;
    message_data_t* data;
    struct icl_entry_s* next;
} icl_entry_t;

typedef struct icl_hash_s {
    int nbuckets;
    int nentries;
    int StorageSize;
    int StorageByteSize;
    int MaxObjSize;
    //per accedere alla repository in mutua esclusione:
    pthread_mutex_t lk_repo;
    int repo_l;
    int fd;// fd della connessione che ha rischiesto la LOCK, -1 come default
    pthread_cond_t cond_repo;
    //per conoscere quanti job stanno lavorando sulla repository:
    pthread_mutex_t lk_job_c;
    int job_c;
    pthread_cond_t cond_job;


    icl_entry_t **buckets;
    icl_entry_lk *lkline;
} icl_hash_t;



icl_hash_t *
icl_hash_create( int nbuckets, int ss, int sbs, int mos);

void freedata(message_data_t* aus);

message_data_t *
icl_hash_find(icl_hash_t *, unsigned long );

int
 icl_hash_insert(icl_hash_t *, unsigned long, message_data_t*);


int
 icl_hash_delete( icl_hash_t *ht, unsigned long key);

int
icl_hash_destroy(icl_hash_t *ht);


int
icl_hash_dump(FILE* stream, icl_hash_t* ht);


#define icl_hash_foreach(ht, tmpint, tmpent, kp, dp)    \
    for (tmpint=0;tmpint<ht->nbuckets; tmpint++)        \
        for (tmpent=ht->buckets[tmpint];                                \
             tmpent!=NULL&&((kp=tmpent->key)!=NULL)&&((dp=tmpent->data)!=NULL); \
             tmpent=tmpent->next)


#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* icl_hash_h */

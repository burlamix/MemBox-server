#include <ops.h>
#include <message.h>
#include <assert.h>
#include <icl_hash.h>

int sendReply(long fd, message_hdr_t *hdr);

int gest_op(message_t* mex,long fd, icl_hash_t* repository,struct statistics  *mboxStats, pthread_mutex_t lk_stat );

int put_op(message_t* mex, icl_hash_t* repository,long fd,struct statistics  *mboxStats, pthread_mutex_t lk_stat );

int update_op(message_t* mex ,icl_hash_t* repository, long fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat );

int remove_op(icl_hash_t* repository, membox_key_t key,long fd,struct statistics  *mboxStats, pthread_mutex_t lk_stat );

int get_op(icl_hash_t* repository, membox_key_t key,long fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat );

int lock_op(long fd,icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat );

int unlock_op(long fd, icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat );
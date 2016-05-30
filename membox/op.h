#include <ops.h>
#include <message.h>
#include <assert.h>
#include <icl_hash.h>

int sendReply(long fd, message_hdr_t *hdr);

int gest_op(message_t mex,long fd, icl_hash_t* repository);

int put_op(char * buff, unsigned int len, icl_hash_t* repository, membox_key_t key,long fd);

int update_op(char * buff, unsigned int len,icl_hash_t* repository, membox_key_t key,long fd);

int remove_op(icl_hash_t* repository, membox_key_t key,long fd);

int get_op(icl_hash_t* repository, membox_key_t key,long fd);

int lock_op(long fd,icl_hash_t* repository);

int unlock_op(long fd, icl_hash_t* repository);
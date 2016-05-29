
int sendReply(long fd, message_hdr_t *hdr);

int gest_op(message_t mex,long fd);

int put_op(char * buff, unsigned int len, memboc_key_t key,long fd);

int update_op(char * buff, unsigned int len, memboc_key_t key,long fd);

int remove_op(memboc_key_t key,long fd);

int get_op(char ** newbuff, unsigned int len, memboc_key_t key,long fd);

int lock_op(long fd);

int unlock_op(long fd);
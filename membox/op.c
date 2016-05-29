#include <ops.h>
#include <message.h>
#include <assert.h>


int gest_op(message_t mex,long fd){

   int ris_op;

	switch (mex.hdr.op) {

   	  case PUT_OP:
   	    		ris_op = put_op(mex.data.buff, mex.data.len, mex.hdr.key,fd);
   	    		break;

 	    case UPDATE_OP:
               ris_op =  update_op(mex.data.buff, mex.data.len, mex.hdr.key,fd);
   	    		break;

       case REMOVE_OP:
               ris_op = get_op(mex.hdr.key,fd);
               break;
   	    		
 	    case GET_OP:
               ris_op = remove_op(newbuff, mex.data.len, mex.hdr.key,fd);
   	    		break;

 	    case LOCK_OP:
   	    		ris_op = lock_op(fd);
   	    		break;

 	    case UNLOCK_OP:
   	    		ris_op = unlock_op(fd);
   	    		break;
	    }

  return ris_op;     
}

void freedata(message_data_t aus) {
  free(aus.buf);
  free(aus);
}

int sendReply(long fd, message_hdr_t *hdr){
  write(fd, &hdr ,sizeof(message_hdr_t));
  return 0;
}

int put_op(char * buff, unsigned int len, memboc_key_t key,fd){
  int op;
  message_data_t dato= malloc(sizeof(message_data_t));
  dato.len=len;
  dato.buff=buff;
  op=icl_hash_insert( repository, (void *) key, (void *) dato);
  //gestione degli errori
  return 0;
}
int update_op(char * buff, unsigned int len, memboc_key_t key,fd){
  message_data_t* dato= (message_data_t*) icl_hash_find( repository, key);
  if(dato!=NULL && dato->len== len){
    //contollo la lunghezza prima di fare un assegnamento 
    dato->buf=buff;
  }else{
    //decidere cosa fare nel caso in cui non ci sia l'oggetto;
  }
	return 0;
}

int remove_op(membox_key_t key,fd){
  int op= icl_hash_delete( repository, key, &free , &freedata );
  //gestione errore se op=-1;
	return 0;
}
int get_op(char ** newbuff, unsigned int len, memboc_key_t key,fd){
	return 0;
}
int lock_op(fd){
    
  pthread_mutex_lock(&lk_conn);
  if(lock_repo){//caso in cui la lock è già presa
      pthread_mutex_unlock(&lk_conn);

      message_hdr_t risp_hdr;
      risp_hdr.op = OP_LOCKED;
      sendReply(/*file_descriptor*/,*risp_hdr)

  }else{

    lock_repo=1;
    pthread_mutex_unlock(&lk_conn);

    message_hdr_t risp_hdr;
    risp_hdr.op = OP_OK;
    sendReply(/*file_descriptor*/,*risp_hdr)
  }
  return 0;
}

int unlock_op(fd){

    pthread_mutex_lock(&lk_conn);
    if(lock_repo){//caso in cui la lock è già presa
        lock_repo=0;
        pthread_mutex_unlock(&lk_conn);

        message_hdr_t risp_hdr;
        risp_hdr.op = OP_OK;
        sendReply(fd,*risp_hdr);

    }else{//caso in cui la lock non è già stata presa
        pthread_mutex_unlock(&lk_conn);

        message_hdr_t risp_hdr;
        risp_hdr.op = OP_LOCK_NONE;
        sendReply(fd,*risp_hdr);
    }
  return 0;
}
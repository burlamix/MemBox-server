#include <ops.h>
#include <message.h>
#include <assert.h>


int gest_op(message_t mex){

   int ris_op;

	switch (mex.hdr.op) {

   	  case PUT_OP:
   	    		ris_op = put_op(mex.data.buff, mex.data.len, mex.hdr.key);
   	    		break;

 	    case UPDATE_OP:
               ris_op =  update_op(mex.data.buff, mex.data.len, mex.hdr.key);
   	    		break;

       case REMOVE_OP:
               ris_op = get_op(mex.hdr.key);
               break;
   	    		
 	    case GET_OP:
               ris_op = remove_op(newbuff, mex.data.len, mex.hdr.key);
   	    		break;

 	    case LOCK_OP:
   	    		ris_op = lock_op();
   	    		break;

 	    case UNLOCK_OP:
   	    		ris_op = unlock_op();
   	    		break;
	    }

  return ris_op;     
}


int put_op(char * buff, unsigned int len, memboc_key_t key){
  int op;
  message_data_t dato= malloc(sizeof(message_data_t));
  dato.len=len;
  dato.buff=buff;
  op=icl_hash_insert( repository, (void *) key, (void *) dato);

  return 0;
}
int update_op(char * buff, unsigned int len, memboc_key_t key){
  
	return 0;
}
int remove_op(memboc_key_t key){
	return 0;
}
int get_op(char ** newbuff, unsigned int len, memboc_key_t key){
	return 0;
}
int lock_op(){
	return 0;
}
int unlock_op(){
	return 0;
}

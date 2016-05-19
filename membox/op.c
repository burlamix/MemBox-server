#include <ops.h>
#include <message.h>
#include <assert.h>


int gest_op(message_t mex){


	switch (mex.hdr.op) {

   	  case PUT_OP:
   	    		put_op(mex.data.buff, mex.data.len, mex.hdr.key);
   	    		break;

 	    case UPDATE_OP:
               update_op(mex.data.buff, mex.data.len, mex.hdr.key);
   	    		break;
   	    		
 	    case GET_OP:
               remove_op(mex.hdr.key);
   	    		break;

 	    case REMOVE_OP:
               get_op(*mex.data.buff, mex.data.len, mex.hdr.key);
   	    		break;

 	    case LOCK_OP:
   	    		statementn;
   	    		break;

 	    case UNLOCK_OP:
   	    		statementn;
   	    		break;

	    case default:
	     		statement;
	     		break;
	    }
}


int put_op(char * buff, unsigned int len, memboc_key_t key){
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
int lock_op(char * buff, memboc_key_t key){
	return 0;
}
int unlock_op(char * buff, memboc_key_t key){
	return 0;
}
int put_op(char * buff, memboc_key_t key){
	return 0;
}
#include <ops.h>
#include <message.h>
#include <assert.h>


int gest_op(message_hdr_t hdr){
	switch (hdr.op) {
   	    case PUT_OP:
   	    		statement1;
   	    		break;

 	    case UPDATE_OP:
   	    		statement2;
   	    		break;
   	    		
 	    case GET_OP:
   	    		statementn;
   	    		break;

 	    case REMOVE_OP:
   	    		statementn;
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
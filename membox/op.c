#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ops.h>
#include <message.h>
#include <assert.h>
#include <icl_hash.h>

int StorageSize= 3000;
int StorageByteSize= 7000;
int MaxObjSize= 100;


void freedata(message_data_t* aus) {
  free(aus->buf);
  free(aus);
}

int sendReply(long fd, message_hdr_t *hdr){
  write(fd, &hdr ,sizeof(message_hdr_t));
  return 0;
}

int SizeOfRep( icl_hash_t* repo){
  int size;
  size= sizeof(icl_hash_t)+ (repo->nbuckets * sizeof( icl_entry_t*)) ;
  size=+ (repo->nentries * sizeof(unsigned long)); //dimensione key
  size=+ (repo->nentries * sizeof(message_data_t)); //dimensione dato
  size=+ (repo->nentries* sizeof(icl_entry_t*)); //dimesione puntatore
  return size;
}

int put_op(char * buff, unsigned int len,icl_hash_t* repository, membox_key_t key,int fd){
  message_hdr_t risp;
  int newdim= SizeOfRep(repository) + (sizeof(unsigned long)) + (sizeof(message_data_t)) + (sizeof(icl_entry_t*));
  //si verifica che la repository non abbia raggiunto il massimo numero di elementi
  if ( repository->nentries >= StorageSize){
    risp.op= OP_PUT_TOOMANY;
    sendReply( fd, &risp);
    return 0;
  }
  //la condizione sulla dimensione della repository viene controllata prima di fare l'inserimento
  if(newdim>= StorageByteSize){
    risp.op= OP_PUT_REPOSIZE;
    sendReply( fd, &risp);
    return 0;
  } 

  int op;
  message_data_t* dato= malloc(sizeof(message_data_t));
  dato->len=len;
  dato->buf=buff;
  if (sizeof(dato)> MaxObjSize){
    free(dato->buf);
    free(dato);
    risp.op= OP_PUT_SIZE;
    sendReply( fd, &risp);
    return 0;
  }
  op=icl_hash_insert( repository, key, (void *) dato);
  switch (op){
    case 0 :
      risp.op= OP_OK;
    case -1 :
    case -3 :
      risp.op= OP_FAIL;
      free(dato->buf);
      free(dato);
    case -2 :
      risp.op= OP_PUT_ALREADY;
      free(dato->buf);
      free(dato);
  }
  sendReply( fd, &risp);
  return 0;
}

int update_op(char * buff, unsigned int len,icl_hash_t* repository, membox_key_t key,int fd){
  message_data_t* dato= (message_data_t*) icl_hash_find( repository, key);
  message_hdr_t risp;
  
  if(dato==NULL){
    risp.op= OP_UPDATE_NONE;
  }
  if (dato->len != len){
    risp.op= OP_UPDATE_SIZE; 
  }else{
    dato->buf=buff;
    risp.op= OP_OK;
  }
  sendReply( fd, &risp);
  return 0;
}

int remove_op(icl_hash_t* repository, membox_key_t key,int fd){
  int op= icl_hash_delete( repository, key, &free , &freedata );
   message_hdr_t risp;
  if (op){
    risp.op= OP_OK;
  }else{
    risp.op=OP_REMOVE_NONE;
  }
    sendReply( fd, &risp);
  return 0;
}
int get_op(icl_hash_t* repository, membox_key_t key,int fd){
    message_data_t* dato= (message_data_t*) icl_hash_find( repository, key);
    message_hdr_t risp;
    if (dato==NULL){
      risp.op=OP_GET_NONE;
    }else{
      risp.op=OP_OK;
    }
    sendReply(fd, &risp);
    write(fd, &(dato->len ),sizeof(int));
  write(fd, dato->buf , dato->len * sizeof(char) );
  return 0;
}

int lock_op(int fd,icl_hash_t* repository){
    
  pthread_mutex_lock(& repository->lk_repo);
  if(repository->repo_l){//caso in cui la lock è già presa
      pthread_mutex_unlock(& repository->lk_repo);

      message_hdr_t risp_hdr;
      risp_hdr.op = OP_LOCKED;
      sendReply( fd, &risp_hdr);

  }else{
    repository->repo_l=1;
    pthread_mutex_unlock(& repository->lk_repo);

    pthread_mutex_lock(&(repository->lk_job_c));
    while(repository->job_c>0){
      pthread_cond_wait(&(repository->cond_job), &(repository->lk_job_c));
    }
    pthread_mutex_unlock(& repository->lk_job_c);

    message_hdr_t risp_hdr;
    risp_hdr.op = OP_OK;
    sendReply( fd , &risp_hdr);
  }
  return 0;
}

int unlock_op(int fd, icl_hash_t* repository){

    pthread_mutex_lock(& repository->lk_repo);
    if(repository->repo_l && fd== repository->fd){//caso in cui la lock è stata presa
        repository->repo_l=0;
        pthread_cond_broadcast(&(repository->cond_repo));
        pthread_mutex_unlock(&(repository->lk_repo));



        message_hdr_t risp_hdr;
        risp_hdr.op = OP_OK;
        sendReply(fd,&risp_hdr);

    }else{//caso in cui la lock non è già stata presa
        pthread_mutex_unlock(& repository->lk_repo);

        message_hdr_t risp_hdr;
        risp_hdr.op = OP_LOCK_NONE;
        sendReply(fd,&risp_hdr);
    }
  return 0;
}



int gest_op(message_t mex,long fd, icl_hash_t* repository){

   int ris_op;

  switch (mex.hdr.op) {

      case PUT_OP:
            ris_op = put_op(mex.data.buf, mex.data.len,repository, mex.hdr.key,fd);
            break;

      case UPDATE_OP:
               ris_op =  update_op(mex.data.buf, mex.data.len, repository, mex.hdr.key,fd);
            break;

       case REMOVE_OP:
               ris_op = remove_op(repository, mex.hdr.key,fd);
               break;
            
      case GET_OP:
               ris_op = get_op( repository, mex.hdr.key,fd);
            break;

      case LOCK_OP:
            ris_op = lock_op(fd, repository);
            break;

      case UNLOCK_OP:
            ris_op = unlock_op(fd,repository);
            break;
      }

  return ris_op;     
}




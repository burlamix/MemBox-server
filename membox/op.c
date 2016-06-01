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



int sendReply(long fd, message_hdr_t *hdr){
  write(fd, hdr ,sizeof(message_hdr_t));
  return 0;
}

int SizeOfRep( icl_hash_t* repo){
  icl_entry_t *bucket, *curr;
    int i,size;

    if(!repo) return -1;

    size=0;
    for(i=0; i<repo->nbuckets; i++) {
        bucket = repo->buckets[i];
        for(curr=bucket; curr!=NULL;curr=curr->next) {
          size=+sizeof(curr->data);  
        }
    }

    return size;
}


int put_op(char * buff, unsigned int len,icl_hash_t* repository, membox_key_t key,int fd){
   message_hdr_t risp;
  int newdim= SizeOfRep(repository) + (sizeof(char)*len) + sizeof(unsigned int);
  //si verifica che la repository non abbia raggiunto il massimo numero di elementi
  if ( repository->StorageSize!=0 && repository->nentries >= repository->StorageSize){
    risp.op= OP_PUT_TOOMANY;
    sendReply( fd, &risp);
    return 0;
  }
  //la condizione sulla dimensione della repository viene controllata prima di fare l'inserimento
  if(repository->StorageByteSize!=0 && newdim>= repository->StorageByteSize){
    risp.op= OP_PUT_REPOSIZE;
    sendReply( fd, &risp);
    return 0;
  } 

  int op;
  message_data_t* dato= malloc(sizeof(message_data_t));
  dato->buf= malloc(sizeof(char)*len);

  dato->len=len;
  dato->buf=buff;

  if (repository->MaxObjSize!=0 && sizeof(dato)> repository->MaxObjSize){
    // free(dato->buf);
    // free(dato);
    risp.op= OP_PUT_SIZE;
    sendReply( fd, &risp);
    return 0;
  }
  op=icl_hash_insert( repository, key, dato);
  switch (op){
    case 0 :
      risp.op= OP_OK;
      break;
    case -1 :
    case -3 :
      risp.op= OP_FAIL;
      // free(dato->buf);
      // free(dato);
      break;
    case -2 :
      risp.op= OP_PUT_ALREADY;
      // free(dato->buf);
      // free(dato);
      break;
  }
  sendReply( fd, &risp);
  return 0;
}

int update_op(char * buff, unsigned int len,icl_hash_t* repository, membox_key_t key,int fd){
  message_data_t* dato= (message_data_t*) icl_hash_find( repository, key);
  message_hdr_t risp;
  
  if(dato==NULL){
    risp.op= OP_UPDATE_NONE;
    sendReply( fd, &risp);
    return 0;
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
  int op= icl_hash_delete( repository, key);
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
      default:
        fprintf(stderr, "Invalid request\n");
        return -1;
      }

  return ris_op;     
}




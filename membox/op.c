#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


int put_op(message_t* new_data,  icl_hash_t* repository,  int fd){
  
  message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));
  int newdim= SizeOfRep(repository) + (new_data->data.len) + sizeof(unsigned int);
  //si verifica che la repository non abbia raggiunto il massimo numero di elementi
  if ( repository->StorageSize!=0 && repository->nentries >= repository->StorageSize){
    risp->op= OP_PUT_TOOMANY;
    sendReply( fd, risp);
    return 0;
  }
  //la condizione sulla dimensione della repository viene controllata prima di fare l'inserimento
  if(repository->StorageByteSize!=0 && newdim>= repository->StorageByteSize){
    risp->op= OP_PUT_REPOSIZE;
    sendReply( fd, risp);
    return 0;
  } 

  int op;

  if (repository->MaxObjSize!=0 && sizeof(new_data->data)> repository->MaxObjSize){
    risp->op= OP_PUT_SIZE;
    sendReply( fd, risp);
    return 0;
  }

  op=icl_hash_insert( repository, new_data->hdr.key, &new_data->data);
  switch (op){
    case 0 :
      risp->op= OP_OK;
      break;
    case -1 :
    case -3 :
      risp->op= OP_FAIL;

      break;
    case -2 :
      risp->op= OP_PUT_ALREADY;

      break;
  }
  sendReply( fd, risp);
  return 0;
}

int update_op(message_t* new_mex, icl_hash_t* repository, int fd){

  message_data_t* dato = (message_data_t*) icl_hash_find( repository, new_mex->hdr.key);
  message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));
  
  if(dato==NULL){
    risp->op= OP_UPDATE_NONE;
    sendReply( fd, risp);
    return 0;
  }
  if (dato->len != new_mex->data.len){
    risp->op= OP_UPDATE_SIZE; 
  }else{
    // free(dato->buf);
    // dato->buf=new_mex->data.buf;      //non sono sicuro al100% che sia la maniera corretta di farlo
    strcpy(dato->buf,new_mex->data.buf);  // qui non va fatto così ma senno
    // free(new_mex);

    risp->op= OP_OK;
  }
  sendReply( fd, risp);
  return 0;
}

int remove_op(icl_hash_t* repository, membox_key_t key,int fd){

  int op= icl_hash_delete( repository, key);
   message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));
  if (op==0){
    risp->op= OP_OK;
  }else{
    risp->op=OP_REMOVE_NONE;
  }
    sendReply( fd, risp);
  return 0;
}

int get_op(icl_hash_t* repository, membox_key_t key,int fd){

    message_data_t* dato;

    dato= (message_data_t*) icl_hash_find( repository, key);

    message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));
    risp->key=key;
    if (dato==NULL){
      risp->op=OP_GET_NONE;
    }else{
      risp->op=OP_OK;
    }
    sendReply(fd, risp);
    write(fd, &(dato->len ),sizeof(int));
    write(fd, dato->buf , dato->len);
  return 0;
}

int lock_op(int fd,icl_hash_t* repository){
  
  message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));

  pthread_mutex_lock(& repository->lk_repo);
  if(repository->repo_l){//caso in cui la lock è già presa
      pthread_mutex_unlock(& repository->lk_repo);

      risp->op = OP_LOCKED;
      sendReply( fd, risp);

  }else{
    repository->repo_l=1;
    pthread_mutex_unlock(& repository->lk_repo);

    pthread_mutex_lock(&(repository->lk_job_c));
    while(repository->job_c>0){
      pthread_cond_wait(&(repository->cond_job), &(repository->lk_job_c));
    }
    pthread_mutex_unlock(& repository->lk_job_c);

    risp->op = OP_OK;
    sendReply( fd , risp);
  }
  return 0;
}

int unlock_op(int fd, icl_hash_t* repository){

  message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));


    pthread_mutex_lock(& repository->lk_repo);
    if(repository->repo_l && fd== repository->fd){//caso in cui la lock è stata presa
        repository->repo_l=0;
        pthread_cond_broadcast(&(repository->cond_repo));
        pthread_mutex_unlock(&(repository->lk_repo));

        risp->op = OP_OK;
        sendReply(fd,risp);

    }else{//caso in cui la lock non è già stata presa
        pthread_mutex_unlock(& repository->lk_repo);

        risp->op = OP_LOCK_NONE;
        sendReply(fd,risp);
    }
  return 0;
}



int gest_op(message_t * mex,long fd, icl_hash_t* repository){

   int ris_op;

  switch (mex->hdr.op) {

      case PUT_OP:
            ris_op = put_op(mex, repository, fd);
            break;

      case UPDATE_OP:
               ris_op =  update_op(mex, repository, fd);
            break;

      case REMOVE_OP:
                ris_op = remove_op(repository, mex->hdr.key,fd);
                break;
            
      case GET_OP:
                ris_op = get_op( repository, mex->hdr.key, fd);
                break;

      // case LOCK_OP:
      //       ris_op = lock_op(fd, repository);
      //       break;

      // case UNLOCK_OP:
      //       ris_op = unlock_op(fd,repository);
      //       break;
      default:
        fprintf(stderr, "Invalid request\n");
        return -1;
      }

  return ris_op;     
}
  
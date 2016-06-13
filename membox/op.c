#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ops.h>
#include <message.h>
#include <assert.h>
#include <icl_hash.h>
#include <err_man.h>
#include <stats.h>

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
          size=size+sizeof(curr->data);  
        }
    }

    return size;
}


int put_op(message_t* new_data,  icl_hash_t* repository,  int fd, struct statistics *mboxStats, pthread_mutex_t lk_stat ){
  
  message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));
  message_data_t *obj=calloc(1,sizeof(message_data_t));
  Pthread_mutex_lock(&lk_stat);
  mboxStats->nput++;
  int newdim= mboxStats->current_size + (new_data->data.len);
  int curr_obj= mboxStats->current_objects;
  Pthread_mutex_unlock(&lk_stat);

  //si verifica che la repository non abbia raggiunto il massimo numero di elementi
  if ( repository->StorageSize!=0 && curr_obj >= repository->StorageSize){
    risp->op= OP_PUT_TOOMANY;
    sendReply( fd, risp);
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nput_failed++;
    Pthread_mutex_unlock(&lk_stat);
    return 0;
  }
  //la condizione sulla dimensione della repository viene controllata prima di fare l'inserimento
  if(repository->StorageByteSize!=0 && newdim> repository->StorageByteSize){
    risp->op= OP_PUT_REPOSIZE;
    sendReply( fd, risp);
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nput_failed++;
    Pthread_mutex_unlock(&lk_stat);
    return 0;
  } 

  int op;

  if (repository->MaxObjSize!=0 && new_data->data.len > repository->MaxObjSize){
    risp->op= OP_PUT_SIZE;
    sendReply( fd, risp);
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nput_failed++;
    Pthread_mutex_unlock(&lk_stat);
    return 0;
  }
  obj->len=new_data->data.len;
  obj->buf=(char*)malloc(obj->len);
  strcpy(obj->buf,new_data->data.buf);
  op=icl_hash_insert( repository, new_data->hdr.key, obj);
  switch (op){
    case 0 :
      risp->op= OP_OK;
      Pthread_mutex_lock(&lk_stat);
      mboxStats->current_size=newdim;
      if(newdim>mboxStats->max_size)
      	mboxStats->max_size=newdim;
      mboxStats->current_objects++;
      if(mboxStats->current_objects > mboxStats->max_objects)
      	mboxStats->max_objects++;
      Pthread_mutex_unlock(&lk_stat);
      break;
    case -1 :
    case -3 :
    	risp->op= OP_FAIL;
      	Pthread_mutex_lock(&lk_stat);
    	mboxStats->nput_failed++;
    	Pthread_mutex_unlock(&lk_stat);
      	break;
    case -2 :
    	risp->op= OP_PUT_ALREADY;
    	Pthread_mutex_lock(&lk_stat);
    	mboxStats->nput_failed++;
    	Pthread_mutex_unlock(&lk_stat);
    	break;
  }
  sendReply( fd, risp);
  return 0;
}

int update_op(message_t* new_mex, icl_hash_t* repository, int fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat){

  message_data_t* dato = (message_data_t*) icl_hash_find( repository, new_mex->hdr.key);
  message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));
  
  if(dato==NULL){
    risp->op= OP_UPDATE_NONE;
    sendReply( fd, risp);
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nupdate++;
    mboxStats->nupdate_failed++;
	Pthread_mutex_unlock(&lk_stat);
    return 0;
  }
  if (dato->len != new_mex->data.len){
    risp->op= OP_UPDATE_SIZE; 
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nupdate++;
    mboxStats->nupdate_failed++;
	Pthread_mutex_unlock(&lk_stat);
  }else{
    // free(dato->buf);
    // dato->buf=new_mex->data.buf;      //non sono sicuro al100% che sia la maniera corretta di farlo
    memcpy((void*)dato->buf,(void*)new_mex->data.buf,dato->len);  // qui non va fatto così ma senno
    // free(new_mex);

    risp->op= OP_OK;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nupdate++;
	Pthread_mutex_unlock(&lk_stat);

  }
  sendReply( fd, risp);
  return 0;
}

int remove_op(icl_hash_t* repository, membox_key_t key,int fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat){

  int op= icl_hash_delete( repository, key);
   message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));
  if (op>=0){
  	risp->op= OP_OK;
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nremove++;
    mboxStats->current_size-=op;
    mboxStats->current_objects--;
	Pthread_mutex_unlock(&lk_stat);
	}else{
    	risp->op=OP_REMOVE_NONE;
    	Pthread_mutex_lock(&lk_stat);
    	mboxStats->nremove++;
    	mboxStats->nremove_failed++;
		Pthread_mutex_unlock(&lk_stat);
  }
    sendReply( fd, risp);
  return 0;
}

int get_op(icl_hash_t* repository, membox_key_t key,int fd, struct statistics  *mboxStats, pthread_mutex_t lk_stat  ){

    message_data_t* dato;

    dato= (message_data_t*) icl_hash_find( repository, key);

    message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));
    risp->key=key;
    if (dato==NULL){
     	risp->op=OP_GET_NONE;
      	Pthread_mutex_lock(&lk_stat);
    	mboxStats->nget++;
    	mboxStats->nget_failed++;
		  Pthread_mutex_unlock(&lk_stat);
    }else{
    	risp->op=OP_OK;
    	Pthread_mutex_lock(&lk_stat);
    	mboxStats->nget++;
		  Pthread_mutex_unlock(&lk_stat);

    }
    sendReply(fd, risp);

    if(risp->op==OP_OK){
      write(fd, &(dato->len ),sizeof(int));
      char * aus;
      aus= dato->buf ;
      int scritti = 0;
      int da_scrivere = dato->len;

      while(da_scrivere>0){
        scritti=write(fd, aus , da_scrivere );

        // printf("\nscrivo----->%d<--\n",scritti );
        aus=aus+scritti;
        da_scrivere=da_scrivere-scritti;
        // printf("********%d************write scrive%d\n",i,r );
      }
      //write(fd, dato->buf , dato->len);
    }
    return 0;
}

int lock_op(int fd,icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat ){
  message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));

  pthread_mutex_lock(& repository->lk_repo);
  if(repository->repo_l){//caso in cui la lock è già presa
      pthread_mutex_unlock(& repository->lk_repo);

      risp->op = OP_LOCKED;
      sendReply( fd, risp);
      Pthread_mutex_lock(&lk_stat);
      mboxStats->nlock++;
      mboxStats->nlock_failed++;
	  Pthread_mutex_unlock(&lk_stat);

  }else{
    repository->repo_l=1;
    repository->fd = fd;
    pthread_mutex_unlock(& repository->lk_repo);

    pthread_mutex_lock(&(repository->lk_job_c));
    while(repository->job_c>1){
      printf("\nvalore di job =%d\n",repository->job_c);

      pthread_cond_wait(&(repository->cond_job), &(repository->lk_job_c));
    }
      printf("\nvalore di job =%d\n",repository->job_c);

    pthread_mutex_unlock(& repository->lk_job_c);

    risp->op = OP_OK;
    sendReply( fd , risp);
    Pthread_mutex_lock(&lk_stat);
    mboxStats->nlock++;
    Pthread_mutex_unlock(&lk_stat);
  }
  return 0;
}

int unlock_op(int fd, icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat ){

  message_hdr_t *risp=calloc(1,sizeof(message_hdr_t));


    pthread_mutex_lock(& repository->lk_repo);
    if(repository->repo_l && fd == repository->fd){//caso in cui la lock è stata presa in maniera corretta
        repository->repo_l=0;
        pthread_mutex_unlock(&(repository->lk_repo));

        risp->op = OP_OK;
        sendReply(fd,risp);
    }else{//caso in cui la lock non è già stata presa o è stata presa da qualcunaltro
        pthread_mutex_unlock(& repository->lk_repo);

        risp->op = OP_LOCK_NONE;
        sendReply(fd,risp);
    }
  return 0;
}



int gest_op(message_t * mex,long fd, icl_hash_t* repository, struct statistics  *mboxStats, pthread_mutex_t lk_stat){

   int ris_op;

  switch (mex->hdr.op) {

      case PUT_OP:
            ris_op = put_op(mex, repository, fd, mboxStats, lk_stat );
            break;

      case UPDATE_OP:
               ris_op =  update_op(mex, repository, fd, mboxStats, lk_stat );
            break;

      case REMOVE_OP:
                ris_op = remove_op(repository, mex->hdr.key,fd, mboxStats, lk_stat );
                break;
            
      case GET_OP:
                ris_op = get_op( repository, mex->hdr.key, fd, mboxStats, lk_stat );
                break;

      case LOCK_OP:
            ris_op = lock_op(fd, repository, mboxStats, lk_stat );
            break;

      case UNLOCK_OP:
            ris_op = unlock_op(fd,repository, mboxStats, lk_stat );
            break;
      default:
        fprintf(stderr, "Invalid request\n");
        return -1;
      }

  return ris_op;     
}
  
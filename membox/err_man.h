#if !defined(MEMBOX_ERRMAN_)
#define MEMBOX_ERRMAN_

#define ec_meno1(s,m) \
	if((s)==-1) { perror (m);} //exit failure ha senso?
#define ec_null(s,m) \
	if((s)==NULL){ perror(m);}

#define ec_meno1_c(s,m,c)\
	if((s)==-1){ perror(m); c;}
#define ec_null_c(s,m,c)\
	if((s)==NULL){ perror(m); c;}


#define Pthread_create(pthread_t* thread_id, const pthread_attr_t *attr, void* (*start_fnc) (void*), void* arg) \
	if((int cod=pthread_create (thread_id, attr,start_fnc, arg))!=0){ perror("error in pthread create:");exit(EXIT_FAILURE);}

#define Pthread_mutex_lock( pthread_mutex_t* mutex) \
	if((int cod=pthread_mutex_lock(mutex))!=0){ perror("error in mutex lock:"); exit(EXIT_FAILURE);}

#define Pthread_mutex_unlock( pthread_mutex_t* mutex)\
	if((int cod=pthread_mutex_unlock(mutex))!=0){ printf("error in mutex unlock:"); exit(EXIT_FAILURE);}

#define Pthread_cond_wait( pthread_cond_t* cond, pthread_mutex_t* mtx) \
	if((int cod=pthread_cond_wait(cond,mtx))!=0){ printf("error in cond wait:"); exit(EXIT_FAILURE);}

#define Pthread_cond_signal( pthread_cond_t* cond)\
	if((int cod=pthread_cond_signal(cond))!=0){ printf("error in cond signal:"); exit(EXIT_FAILURE);}


//atexit da registrare
#endif /* MEMBOX_ERRMAN_ */

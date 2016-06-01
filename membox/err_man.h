#if !defined(MEMBOX_ERRMAN_)
#define MEMBOX_ERRMAN_

#define ec_meno1(s,m) \
	if((s)==-1) { perror (m);} //exit failure ha senso?
#define ec_null(s,m) \
	if((s)==NULL){ perror(m);}

#define ec_meno1_np(s,c)\
	if((s)==-1) { c;}
#define ec_null_np(s,c)\
	if((s)==NULL){c;}

#define ec_meno1_ex(s,m) \
	if((s)==-1) { perror (m); exit(EXIT_FAILURE); } //exit failure ha senso?
#define ec_null_ex(s,m) \
	if((s)==NULL){ perror(m); exit(EXIT_FAILURE);}

#define ec_meno1_c(s,m,c)\
	if((s)==-1){ perror(m); c;}
#define ec_null_c(s,m,c)\
	if((s)==NULL){ perror(m); c;}

//quella che da problemi

#define ec_meno1_r(s,m,c)\
	if((s)==-1){ perror(m); c;}\
	if((s)==0){ printf("\n\n\nciao\n\n\n\n");return 0;}


#define Pthread_create(thread_id, attr, start_fnc, arg) \
	if((int cod=pthread_create (thread_id, attr,start_fnc, arg))!=0){ perror("error in pthread create:");exit(EXIT_FAILURE);}

#define Pthread_mutex_lock( mutex) \
	if((pthread_mutex_lock(mutex))!=0){ perror("error in mutex lock:"); exit(EXIT_FAILURE);}

#define Pthread_mutex_unlock( mutex)\
	if((pthread_mutex_unlock(mutex))!=0){ printf("error in mutex unlock:"); exit(EXIT_FAILURE);}

#define Pthread_cond_wait( cond, mtx) \
	if((pthread_cond_wait(cond,mtx))!=0){ printf("error in cond wait:"); exit(EXIT_FAILURE);}

#define Pthread_cond_signal( cond)\
	if((pthread_cond_signal(cond))!=0){ printf("error in cond signal:"); exit(EXIT_FAILURE);}

int cleanall(void*);
//atexit da registrare
#endif /* MEMBOX_ERRMAN_ */

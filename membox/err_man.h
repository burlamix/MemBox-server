#if !defined(MEMBOX_ERRMAN_)
#define MEMBOX_ERRMAN_
#include <pthread.h>


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

void Pthread_mutex_lock(pthread_mutex_t* mutex);
void Pthread_mutex_unlock(pthread_mutex_t* mutex); 
void Pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex); 
void Pthread_cond_signal(pthread_cond_t* cond); 


int cleanall(void*);
//atexit da registrare
#endif /* MEMBOX_ERRMAN_ */

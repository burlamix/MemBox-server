#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

#include "err_man.h"


/**
 * @function Pthread_mutex_lock
 * @brief tenta per 10 volte di fare la lock, e se non riesce in meno di 10 volte, viene mandato un segnale che fa terminare il server
 *
 * @param mutex     variabile di mutua esclusione
 *
 */
void Pthread_mutex_lock(pthread_mutex_t* mutex) {

	int err = 0;
	int i = 0;
	while ( (err = pthread_mutex_lock(mutex)) != 0 && i < 10) {
		i++;
	}

	if (i == 10) {
		abort();
	}
}
/**
 * @function Pthread_mutex_unlock
 * @brief tenta per 10 volte di fare la unlock, e se non riesce in meno di 10 volte, viene mandato un segnale che fa terminare il server
 *
 * @param mutex     variabile di mutua esclusione
 *
 */
void Pthread_mutex_unlock(pthread_mutex_t* mutex) {

	int err = 0;
	int i = 0;
	while ( (err = pthread_mutex_unlock(mutex)) != 0 && i < 10) {
		i++;
	}

	if (i == 10) {
		abort();
		//printf("error in mutex unlock:%d",err);
		//ec_meno1(kill(0, SIGUSR2),"kill");
	}
}
/**
 * @function Pthread_cond_wait
 * @brief tenta per 10 volte di fare la wait, e se non riesce in meno di 10 volte, viene mandato un segnale che fa terminare il server
 *
 * @param cond     variabile di condizione
 * @param mutex     variabile di mutua esclusione
 *
 */
void Pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
	if (pthread_cond_wait(cond, mutex) != 0)
		abort();

}
/**
 * @function Pthread_cond_signal
 * @brief tenta per 10 volte di fare la signal, e se non riesce in meno di 10 volte, viene mandato un segnale che fa terminare il server
 *
 * @param mutex     variabile di condizione
 *
 */
void Pthread_cond_signal(pthread_cond_t* cond) {
	if (pthread_cond_signal(cond) != 0) {
		abort();
	}
}

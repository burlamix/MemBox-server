/*
 * membox Progetto del corso di LSO 2016 
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Pelagatti, Torquati
 * 
 */
/**
 * @file membox.c
 * @brief File principale del server membox
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

/* inserire gli altri include che servono */

#include <stats.h>


#define N 100
#define TRUE 1
#define UNIX_PATH_MAX 108
#define SOCKNAME "sock_server"
/* struttura che memorizza le statistiche del server, struct statistics 
 * e' definita in stats.h.
 *
 */
struct statistics  mboxStats = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
//_______________________________________________________________________________________________________strutture_________________________________

typedef struct nodo 
{
	int info;
	struct nodo * next;
}nodo;

typedef nodo * lista_fd;




//_________________________________________________________________________________________________variabili_di _codizione_______________________________________

pthread_mutex_t lk_tatt = PTHREAD_MUTEX_INITIALIZER;
int tatt=0;
pthread_cond_t cond_tatt = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lk_lostsignal = PTHREAD_MUTEX_INITIALIZER;
int lost_signal=0;

pthread_mutex_t lk_con_attive = PTHREAD_MUTEX_INITIALIZER;
con_attive = 0;

lista_fd l_fd=NULL;

//_____________________________________________________________________________cose da finire___________________________________________________________

int maxconnection=20;

//________________________________________________________________________________________________________________________________________


void * dispatcher()
{
	printf("\n DISPATCHER PARTITO \n");fflush(stdout);

	int fd_skt, fd_c;
	struct sockaddr_un sa;

	(void) unlink(SOCKNAME);
	strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);				/* sistemo l'indirizzo */
	sa.sun_family = AF_UNIX;

	fd_skt=socket(AF_UNIX,SOCK_STREAM,0);						/* preparo la socket */
	bind( fd_skt , (struct sockaddr *) &sa , sizeof(sa) );
	listen(fd_skt,SOMAXCONN);

	while(TRUE){

		pthread_mutex_lock(&lk_con_attive);
		if(maxconnection - con_attive){
		pthread_mutex_unlock(&lk_con_attive);

					fd_c = accept(fd_skt,NULL,0);

		pthread_mutex_lock(&lk_tatt);
			//sospeso la messa in coda del file descriptor. 
		pthread_cond_signal(&lk_tatt);


		}

		fd_c = accept(fd_skt,NULL,0);

		pthread_mutex_lock(&mtx);

			fd_cm=fd_c;
		
		pthread_cond_signal(&con);	
		pthread_mutex_unlock(&mtx);
		i++;
	}


	close(fd_skt);
	unlink(SOCKNAME);


	pthread_exit((void *) 0);
}


int main(int argc, char *argv[]) {

    return 0;
}

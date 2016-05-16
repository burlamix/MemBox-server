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
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <message.h>
#include "liste.c"    //momentaneo
// #include <liste.h>

#define TRUE 1
#define FALSE 0
#define UNIX_PATH_MAX 108
#define SOCKNAME "sock_server"
/* struttura che memorizza le statistiche del server, struct statistics 
 * e' definita in stats.h.
 *
 */
struct statistics  mboxStats = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
//_______________________________________________________________________________________________________strutture_________________________________





//_________________________________________________________________________________________________variabili_di _codizione_______________________________________

pthread_cond_t cond_nuovolavoro = PTHREAD_COND_INITIALIZER;


pthread_mutex_t lk_conn = PTHREAD_MUTEX_INITIALIZER;

coda_fd* coda_conn= initcoda();

// lista_fd l_fd=NULL;

//_____________________________________________________________________________cose da finire___________________________________________________________

int maxconnection=20;
int ThreadsInPool=5;

//________________________________________________________________________________________________________________________________________


void * dispatcher()
{
	printf("\n DISPATCHER PARTITO \n");fflush(stdout);

	int fd_skt, fd_c;
	struct sockaddr_un sa;



	(void) unlink(SOCKNAME);							//notare che le tre righe successive sono ricorrenti anche in connection.c si potrebbe fare una funzione he le chiama?
	strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);				/* sistemo l'indirizzo */
	sa.sun_family = AF_UNIX;

	fd_skt=socket(AF_UNIX,SOCK_STREAM,0);						/* preparo la socket */
	bind( fd_skt , (struct sockaddr *) &sa , sizeof(sa) );
	listen(fd_skt,SOMAXCONN);

	while(TRUE){

		pthread_mutex_lock(&lk_conn);
			if(maxconnection-(coda_conn->lenght)>0){
				fd_c=accept(fd_skt,NULL,0);
				add_fd(coda_conn,fd_c);
				pthread_cond_signal(&cond_nuovolavoro);
			}
		pthread_mutex_unlock(&lk_conn);

	}

	close(fd_skt);
	unlink(SOCKNAME);

	pthread_exit((void *) 0);
	
}

void* worker(){
	nodo* job;
	int fd;
	message_t dati;

	while(1){
	pthread_mutex_lock(&lk_conn);
		while(coda_conn->testa_attesa==NULL){//fare con una funzione
			pthread_cond_wait(&cond_nuovolavoro,&lk_conn);
			pthread_mutex_unlock(&lk_conn);
			
		}
		// fare con una funzione
		job=coda_conn->testa_attesa;
		fd=coda_conn->testa_attesa->info;
		coda_conn->testa_attesa=coda_conn->testa_attesa->next;
		pthread_mutex_unlock(&lk_conn);
		
		read(fd, &dati.hdr ,sizeof(message_hdr_t));
		read(fd, &dati.data.len ,sizeof(int));
		dati.data.buf = malloc(sizeof(char)*dati.data.len);
		read(fd, dati.data.buf ,dati.data.len *sizeof(char));	
		
		

	pthread_mutex_lock(&lk_conn);
	delete_fd(coda_conn, job);
	pthread_mutex_unlock(&lk_conn);

		
	}		
		
	
}


int main(int argc, char *argv[]) {
	pthread_t threadinpool[ThreadsInPool];
	pthread_t disp;
	pthread_create(&disp, NULL, dispatcher,NULL);
	for(int i=0;i<ThreadsInPool;i++){
		pthread_create(&threadinpool[i],NULL,worker, NULL);
	}
	


    return 0;
}


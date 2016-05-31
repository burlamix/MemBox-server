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
#include <sys/un.h>
#include <message.h>
#include "liste.c"    //momentaneo
#include <icl_hash.h>
#include <connections.h>
#include <op.h>
#include <parse.h>



// #include <liste.h>

#define TRUE 1
#define FALSE 0
#define NB 1013
#define SOCKNAME "/tmp/mbox_socket"
/* struttura che memorizza le statistiche del server, struct statistics 
 * e' definita in stats.h.
 *
 */
struct statistics  mboxStats = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
//_______________________________________________________________________________________________________strutture_________________________________





//_________________________________________________________________________________________________variabili_di _codizione_______________________________________

pthread_cond_t cond_nuovolavoro = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lk_conn = PTHREAD_MUTEX_INITIALIZER;
coda_fd* coda_conn;

//pthread_cond_t cond_repo = PTHREAD_COND_INITIALIZER;
//pthread_mutex_t lk_repo = PTHREAD_MUTEX_INITIALIZER;
//int repo_l=0;

//pthread_mutex_t lk_job_c = PTHREAD_MUTEX_INITIALIZER;
//int job_c=0;

icl_hash_t * repository; 


// lista_fd l_fd=NULL;

//_____________________________________________________________________________cose da finire___________________________________________________________


var_conf v_configurazione;


//________________________________________________________________________________________________________________________________________






void *dispatcher()
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

			if( v_configurazione.MaxConnections-(coda_conn->lenght)>0){ // se posso ancora accettare connessioni posso non accederci in lock perchè i woker possono solo decrementare il valore

				printf("	accept prima \n");	fflush(stdout);
			fd_c=accept(fd_skt,NULL,0);

				printf("	accept dopo \n");	fflush(stdout);

				printf("	infilo prima \n");	fflush(stdout);

				pthread_mutex_lock(&lk_conn);
			add_fd(coda_conn,fd_c);
				printf("	infilo dopo \n");	fflush(stdout);

				pthread_cond_signal(&cond_nuovolavoro);
				pthread_mutex_unlock(&lk_conn);
			}
			else{								//ramo da controllarne il funzionamento correto
				
				fd_c=accept(fd_skt,NULL,0);
				message_t fail;
				fail.hdr.op = OP_FAIL; 			//da definire poi un apposito messaggio per il numero massimo di connessioni raggiunto
				sendRequest(fd_c, &fail); 
			}

	}

	close(fd_skt);
	unlink(SOCKNAME);

	pthread_exit((void *) 0);
	
}

void* worker(){

	printf("\n WORKER PARTITO \n");fflush(stdout);

	nodo* job;
	int fd;
	int ris_op;
	message_t dati;

	while(1){
	pthread_mutex_lock(&lk_conn);
		while(coda_conn->testa_attesa==NULL){//fare con una funzione
			printf("	testa attesa è nulla \n");	fflush(stdout);

			pthread_cond_wait(&cond_nuovolavoro,&lk_conn);
		}
		printf("!!!!!!!!!!!!!!!!	worker \n");	fflush(stdout);

		// fare con una funzione
		job=coda_conn->testa_attesa;
		fd=coda_conn->testa_attesa->info;
		coda_conn->testa_attesa=coda_conn->testa_attesa->next;
		pthread_mutex_unlock(&lk_conn);
		
		

		while(1)// e poi ci infiliamo una read
		{	printf("!!!!!!!!!!!!!!!!	worker \n");	fflush(stdout);
			read(fd, &dati.hdr ,sizeof(message_hdr_t));					//da implementare connnnn una fun, che fa tutto e gestisce quando la read sengla che il fd è stato chiuso
			read(fd, &dati.data.len ,sizeof(int));
			dati.data.buf = malloc(sizeof(char)*dati.data.len);
			read(fd, dati.data.buf ,dati.data.len *sizeof(char));	
		
			pthread_mutex_lock(&(repository->lk_repo));
				while(repository->repo_l == 1){
					pthread_cond_wait(&(repository->cond_repo),&(repository->lk_repo));
				}
			pthread_mutex_lock(&(repository->lk_job_c));
				repository->job_c++;							// !!!*!*!!! questo non va messo subito dopo il while sopra?
			pthread_mutex_unlock(&repository->lk_job_c);
			pthread_mutex_unlock(&(repository->lk_repo));

			printf("lavoro \n");	fflush(stdout);

			ris_op = gest_op(dati,fd, repository);

			//liberare memoria del messaggio
			pthread_mutex_lock(&(repository->lk_job_c));
				repository->job_c--;
				//nel caso in cui non ci siano più lavori in esecuzione allora viene attivata la lock
				if(repository->job_c==0) 
					pthread_cond_signal(&(repository->cond_job));
			pthread_mutex_unlock(&(repository->lk_job_c));
	
		}	
		pthread_mutex_lock(&lk_conn);
		delete_fd(coda_conn, job);
		pthread_mutex_unlock(&lk_conn);

	}		
}


int main(int argc, char *argv[]) {

	pthread_t *threadinpool;
	pthread_t disp;
	coda_conn=initcoda();

	parse(argv[2], &v_configurazione);
	threadinpool = malloc(v_configurazione.ThreadsInPool*(sizeof(pthread_t)));

	repository = icl_hash_create( NB, NULL,NULL);

	pthread_create(&disp, NULL, dispatcher,NULL);
	for(int i=0;i<v_configurazione.ThreadsInPool;i++){
		pthread_create(&threadinpool[i],NULL,worker, NULL);
	}
	
	pthread_join(disp,NULL);


    return 0;
}


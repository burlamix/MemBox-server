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
#include <liste.h>  
#include <icl_hash.h>
#include <connections.h>
#include <op.h>
#include <parse.h>
#include <err_man.h>


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

//coda dei fd relativi al socket
coda_fd* coda_conn;

//repository
icl_hash_t * repository; 

//_________________________________________________________________________________________________variabili_di _codizione_______________________________________
//variabili di mutua esclusione e di condizione per l'accesso alla coda dei fd relativi al socket
pthread_cond_t cond_nuovolavoro = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lk_conn = PTHREAD_MUTEX_INITIALIZER;



//_____________________________________________________________________________cose da finire___________________________________________________________


var_conf v_configurazione;


//________________________________________________________________________________________________________________________________________






void *dispatcher()
{
	printf("\n DISPATCHER PARTITO \n");fflush(stdout);// da eliminare

	int fd_skt, fd_c;
	struct sockaddr_un sa;



	(void) unlink(SOCKNAME);															// sistemo l'indirizzo 
	ec_null_ex(strncpy(sa.sun_path, v_configurazione.UnixPath,UNIX_PATH_MAX), "impossibile creare path");		 
	sa.sun_family = AF_UNIX;

	ec_meno1_ex(fd_skt=socket(AF_UNIX,SOCK_STREAM,0),"impossibile creare socket");		// preparo il socket 
	ec_meno1_ex(bind( fd_skt , (struct sockaddr *) &sa , sizeof(sa) ), "impossibile assegnare indirizzo a socket");
	ec_meno1_ex(listen(fd_skt,SOMAXCONN),"impossibile accettare connessioni");

	while(TRUE){
			Pthread_mutex_lock(&lk_conn);
			if( v_configurazione.MaxConnections-(coda_conn->lenght)>0){ //si possono ancora accettare connessioni
				Pthread_mutex_unlock(&lk_conn);
				//fallisce una connessione e buttiamo giù tutto?
				ec_meno1_c(fd_c=accept(fd_skt,NULL,0), "connessione fallita", break);

				printf("\n\nnuova connessione accettata -> si procede all'inserimento del fd nella lista\n");

				Pthread_mutex_lock(&lk_conn);
				add_fd(coda_conn,fd_c);
				Pthread_cond_signal(&cond_nuovolavoro);
				Pthread_mutex_unlock(&lk_conn);
			}
			else{// impossibile accettare nuove connessioni connessioni
				Pthread_mutex_unlock(&lk_conn);
				ec_meno1_c(fd_c=accept(fd_skt,NULL,0),"connessione fallita", break);
				message_t fail;
				fail.hdr.op = OP_FAIL; 			//da definire poi un apposito messaggio per il numero massimo di connessioni raggiunto
				ec_meno1(sendRequest(fd_c, &fail),"impossibile inviare messaggio");  
			}

	}

	ec_meno1_ex(close(fd_skt),"impossibile chiudere socket");
	ec_meno1_ex(unlink(SOCKNAME),"errore di unlink");

	pthread_exit((void *) 0);
	
}
// momentaneamente l'ho messa qui, dove altro si può mettere sennò?
int read_fd (int fd,message_t* dati){
	
	//il worker legge il messaggio 

	if (read(fd, &dati->hdr ,sizeof(message_hdr_t)) == 0)  return 0; 		// qui è dove c'ho perso più di un ora

	ec_meno1_c(read(fd, &dati->data.len ,sizeof(int)),"errore lettura dati messaggio", NULL);
	ec_null_c(dati->data.buf = malloc(sizeof(char)*dati->data.len),"errore allocazione dato", NULL);
	ec_meno1_c(read(fd, dati->data.buf ,dati->data.len *sizeof(char)),"errore lettura contenuto messaggio", free(dati->data.buf); NULL);	

	return 1;
}

void* worker(){

	printf("\n WORKER PARTITO \n");fflush(stdout); //da eliminare

	nodo* job;
	int fd;
	int i;
	int ris_op;
	message_t dati;

	while(1){
	Pthread_mutex_lock(&lk_conn);
		while(coda_conn->testa_attesa==NULL){//verifica la presenza di nuovi lavori
			printf("	testa attesa è nulla -> il worker si sospende\n");	fflush(stdout);

			Pthread_cond_wait(&cond_nuovolavoro,&lk_conn);
		}

		printf("\n worker si sveglia e procede a prendere una nuova connessione,");	fflush(stdout);
		// fare con una funzione
		job=coda_conn->testa_attesa;
		fd=coda_conn->testa_attesa->info;
		coda_conn->testa_attesa=coda_conn->testa_attesa->next;
		Pthread_mutex_unlock(&lk_conn);
		printf(" l'fd della nuova connessione è %d\n",fd);	fflush(stdout);
		
		i=0;
		while(read_fd(fd,&dati))// e poi ci infiliamo una read
		{	

			printf("\n i=%d, worker esegue una richiesta di %d del client su fd = %d \n",i,dati.hdr.op,fd);	fflush(stdout);
			

			//controlla che la repository non sia bloccata da una operazione LOCK
			Pthread_mutex_lock(&(repository->lk_repo));
				while(repository->repo_l == 1){
					Pthread_cond_wait(&(repository->cond_repo),&(repository->lk_repo));
				}
			//se la repository non è bloccata viene incrementato il valore che identifica il numero di operazioni in esecuzione sulla repository
			Pthread_mutex_lock(&(repository->lk_job_c));
				repository->job_c++;							// !!!*!*!!! questo non va messo subito dopo il while sopra?
			Pthread_mutex_unlock(&repository->lk_job_c);
			Pthread_mutex_unlock(&(repository->lk_repo));

			//viene eseguita l'operazione richiesta
			ec_meno1_c(ris_op = gest_op(dati,fd, repository), "operazione non identificata", free(dati.data.buf);break);

			free(dati.data.buf);
			Pthread_mutex_lock(&(repository->lk_job_c));
				repository->job_c--;
				//nel caso in cui non ci siano più lavori in esecuzione allora viene attivata la lock
				if(repository->job_c==0) 
					Pthread_cond_signal(&(repository->cond_job));//nel caso nessuno abbia chiesto la lock la signal andrà persa
			Pthread_mutex_unlock(&(repository->lk_job_c));
	
		}	
		Pthread_mutex_lock(&lk_conn);
		delete_fd(coda_conn, job);
		Pthread_mutex_unlock(&lk_conn);

	}		
}


int main(int argc, char *argv[]) {

	pthread_t *threadinpool;
	pthread_t disp;
	coda_conn=initcoda();

	parse(argv[2], &v_configurazione);
	threadinpool = malloc(v_configurazione.ThreadsInPool*(sizeof(pthread_t)));

	repository = icl_hash_create( NB, v_configurazione.StorageSize, v_configurazione.StorageByteSize, v_configurazione.MaxObjSize);

	pthread_create(&disp, NULL, dispatcher,NULL);
	for(int i=0;i<v_configurazione.ThreadsInPool;i++){
		pthread_create(&threadinpool[i],NULL,worker, NULL);
	}
	
	pthread_join(disp,NULL);


    return 0;
}


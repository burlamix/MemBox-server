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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define TRUE 1
#define FALSE 0
#define NB 1013
/* struttura che memorizza le statistiche del server, struct statistics 
 * e' definita in stats.h.
 *
 */
struct statistics  mboxStats = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
//file delle statistiche
FILE* file_stat;

//coda di file descriptor utilizzata per la sincronizzazione dei vari thread worker e dispatcher
coda_fd* coda_conn;

//repository
icl_hash_t * repository; 

//variabili di mutua esclusione e di condizione per l'accesso alla coda dei fd relativi al socket
pthread_cond_t cond_nuovolavoro = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lk_conn = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t lk_stat = PTHREAD_MUTEX_INITIALIZER;


// struttura delle variabili passate da parse
var_conf v_configurazione;

//variabili e_flag che servono per sincronizzare sig_handler con tutti gli altri thread
//serve per far terminare tutto le richieste di un client a un worker e poi teminare
volatile sig_atomic_t e_flag = 0;
//serve per far terminare immediatamente i woker una volta terminata l'operazione
volatile sig_atomic_t i_flag = 0;

//il fd della socket come variabile globale perchè è necessario bloccarla quando arriva segnale sigusr2
int fd_skt;




/**
 * @function dispatcher
 * @brief gestisce le richieste di connessione da parte dei client
 *
 * */
void *dispatcher()
{
	printf("\n DISPATCHER PARTITO \n");fflush(stdout);// da eliminare


	int fd_c, aus;
	struct sockaddr_un sa;


	(void) unlink(v_configurazione.UnixPath);	

	// sistemo la soket
	ec_null_ex(strncpy(sa.sun_path, v_configurazione.UnixPath,UNIX_PATH_MAX), "impossibile creare path");		 
	sa.sun_family = AF_UNIX;
	ec_meno1_ex(fd_skt=socket(AF_UNIX,SOCK_STREAM,0),"impossibile creare socket");		// preparo il socket 
	ec_meno1_ex(bind( fd_skt , (struct sockaddr *) &sa , sizeof(sa) ), "impossibile assegnare indirizzo a socket");
	ec_meno1_ex(listen(fd_skt,SOMAXCONN),"impossibile accettare connessioni");

	//finche non è stato mandato un sengnale di terminazione continuo ad accettare connessioni
	while(e_flag==0 && i_flag==0){

			aus=0;
			ec_meno1_c(fd_c=accept(fd_skt,NULL,0), "connessione fallita", break);

			Pthread_mutex_lock(&lk_conn);

			//si possono ancora accettare connessioni
			if( v_configurazione.MaxConnections-(coda_conn->lenght)>0){ 
				Pthread_mutex_unlock(&lk_conn);

				//fallisce una connessione e buttiamo giù tutto?
				printf("\n\nnuova connessione accettata -> si procede all'inserimento del fd nella lista\n");

				Pthread_mutex_lock(&lk_conn);
				ec_meno1_np(add_fd(coda_conn,fd_c), aus=-1);
				Pthread_cond_signal(&cond_nuovolavoro);
				Pthread_mutex_unlock(&lk_conn);
			}
			else{// impossibile accettare nuove connessioni connessioni
				Pthread_mutex_unlock(&lk_conn);
				aus=-1;
			}
			if(aus!=0){
				// se il numero di connessioni gestibili è raggiunto rispondo con operazione fallita
				message_t fail;
				fail.hdr.op = OP_FAIL; 			
				ec_meno1(sendRequest(fd_c, &fail),"impossibile inviare messaggio");  
			}

	}

	ec_meno1_ex(close(fd_skt),"impossibile chiudere socket");
	ec_meno1_ex(unlink(v_configurazione.UnixPath),"errore di unlink");

	//sveglio tutti i worker che sono in attesa sulla coda delle connessioni così che si accorgano che devono teminare
	Pthread_mutex_lock(&lk_conn);
	pthread_cond_broadcast(&cond_nuovolavoro);
	Pthread_mutex_unlock(&lk_conn);


	printf("-----il dispatcher muore-----\n");
	fflush(stdout);

	pthread_exit((void *) 0);
	
}

/**
 * @function worker
 * @brief gestisce le richieste di operazione dei client
 *
 * */
void* worker(){

	printf("\n WORKER PARTITO \n");fflush(stdout); //da eliminare

	nodo* job;
	int fd;
	int i;
	int ris_op;
	//struttura che serve per riceverei i messaggi
	message_t *dati =malloc(sizeof(message_t));


	while(i_flag==0){
	Pthread_mutex_lock(&lk_conn);
		//nel finche non è settato uno dei due flag o non c'è nessun lavoro da svolgere attende
		while(e_flag==0 && i_flag==0 && coda_conn->testa_attesa==NULL){//verifica la presenza di nuovi lavori
			printf("testa attesa è nulla -> il worker si sospende\n");	fflush(stdout);

			Pthread_cond_wait(&cond_nuovolavoro,&lk_conn);
		}

		// se viene svegliato ma uno dei due flag è settato, significa che deve terminare quindi interrompe il ciclo
		if(e_flag!=0 && i_flag!=0) {Pthread_mutex_unlock(&lk_conn);break;}

		printf("\n\n\n\n__________________________________inizio______________________________________________________________________");	fflush(stdout);

		printf("\nworker si sveglia e procede a prendere una nuova connessione,");	fflush(stdout);
		// fare con una funzione
		job=coda_conn->testa_attesa;
		//se job==NULL vuol dire che sono uscito dal ciclo per volatile e le connessioni sono finite quindi esco dal while;
		if((e_flag!=0 && job==NULL) || i_flag!=0){
			Pthread_mutex_unlock(&lk_conn);
			break;
		}
		fd=coda_conn->testa_attesa->info;
		coda_conn->testa_attesa=coda_conn->testa_attesa->next;
		mboxStats.concurrent_connections++;
		Pthread_mutex_unlock(&lk_conn);
		printf(" l'fd della nuova connessione è %d\n",fd);	fflush(stdout);
		

		i=0;
		//finche ricevo una richiesta e non è settato il flag continuo a eseguire le richieste del client
		while( i_flag==0 && readHeader(fd, &dati->hdr))// e poi ci infiliamo una read
		{	
			//se la richesta è di put e di update leggo la parte data del messaggio
			if(dati->hdr.op == PUT_OP || dati->hdr.op == UPDATE_OP ){
				readData(fd,&dati->data);
			}else{
				printf("********************read non legge! \n" );
			}
					
			printf("i=%d, worker esegue una richiesta di %d del client su fd = %d, ",i,dati->hdr.op,fd);	fflush(stdout);
			

			//controlla che la repository non sia bloccata da una operazione LOCK
			Pthread_mutex_lock(&(repository->lk_repo));
				if(repository->repo_l == 0 || repository->fd == fd){
				
				//se la repository non è bloccata viene incrementato il valore che identifica il numero di operazioni in esecuzione sulla repository
				Pthread_mutex_lock(&(repository->lk_job_c));
					repository->job_c++;							
				Pthread_mutex_unlock(&repository->lk_job_c);
				Pthread_mutex_unlock(&(repository->lk_repo));

				//viene eseguita l'operazione richiesta
				ec_meno1_c(ris_op = gest_op(dati,fd, repository, &mboxStats, lk_stat), "operazione non identificata", free(dati->data.buf);break);
				printStats(stdout);
				printf(" risultato dell op=%d\n",ris_op);

				//posso liberare il buffer che verrà riallocato nuovamente alla prossima readData
				if(dati->hdr.op == PUT_OP || dati->hdr.op == UPDATE_OP ){
					free(dati->data.buf);
				}	
					

				Pthread_mutex_lock(&(repository->lk_job_c));
					repository->job_c--;

					//nel caso in cui non ci siano più lavori in esecuzione allora viene attivata la lock
					if(repository->job_c==0) 
						Pthread_cond_signal(&(repository->cond_job));//nel caso nessuno abbia chiesto la lock la signal andrà persa
				Pthread_mutex_unlock(&(repository->lk_job_c));
				i++;

			}else{
				//se la repository è bloccata rispondo con l'opportuno messaggio
				Pthread_mutex_unlock(&(repository->lk_repo));
 				message_hdr_t *risp=malloc(sizeof(message_hdr_t));
				risp->op = OP_LOCKED;
       			sendReply(fd,risp);

			}
		}	
		//terminate le richieste del client elimino il fd dall coda e aggiorno le statistiche
		Pthread_mutex_lock(&lk_conn);
		delete_fd(coda_conn, job);
		mboxStats.concurrent_connections--;
		Pthread_mutex_unlock(&lk_conn);
		
		//printf("\n-----------------------------dump----------------------------------\n");
  	 	 //icl_hash_dump(stdout, repository);
		//printf("\n------------------------------------------------------------------\n");
 
		printf("\n_____________________________________________fine___________________________________________________________\n\n");	fflush(stdout);

	}
	free(dati);
	printf("-----il worker muore-----\n");
	fflush(stdout);
	pthread_exit((void *) 0);		
}

/**
 * @function sig_handler
 * @brief gestisce i segnali
 *
 * */
void* sig_handler(){
	sigset_t set;
	int sig, aus_sig;
	sigemptyset(&set);
	sigaddset(&set,SIGTERM);
	sigaddset(&set,SIGQUIT);
	sigaddset(&set,SIGINT);
	sigaddset(&set,SIGUSR1);
	sigaddset(&set,SIGUSR2);
	pthread_sigmask(SIG_SETMASK,&set,NULL);
	aus_sig=1;
	while(aus_sig){
		sigwait(&set,&sig);
		printf(" SEGNALE:%d",sig);fflush(stdout);
		switch (sig) {
			case SIGUSR1: {
				if (file_stat!=NULL) printStats(file_stat);
				break;
			}
			case SIGUSR2: {
				e_flag=1;
				shutdown(fd_skt,SHUT_RDWR); //andrea quarta ha detto che non va bene
				printf("%d -------------------------------------\n\n\n",e_flag);
				fflush(stdout);
				printf("-----handler si sveglia e elimina tutto----\n");
				fflush(stdout);
				
				if (file_stat!=NULL) printStats(file_stat);

				aus_sig=0;
				break;
			}
			default: { //forse è meglio specificare i casi dei diversi segnali
				i_flag=1;
				shutdown(fd_skt,SHUT_RDWR);
				aus_sig=0;
				break;

			}
			
		}
	}
	printf("-----handler si chiude----\n");
	fflush(stdout);
	pthread_exit((void *) 0);

}



/**
 * @function main
 *
 * @brief gestisce le strutture globali e la creazione dei thread
 *
 * */
int main(int argc, char *argv[]) {
	sigset_t set;
	
	/*maschero tutto i segnali*/
	//ec_meno1_np(sigfillset(&set), exit(EXIT_FAILURE));
	//ec_meno1_np(pthread_sigmask(SIG_SETMASK, &set,NULL),exit(EXIT_FAILURE));

	sigemptyset(&set);
	sigaddset(&set,SIGTERM);
	sigaddset(&set,SIGTERM);
	sigaddset(&set,SIGQUIT);
	sigaddset(&set,SIGINT);
	sigaddset(&set,SIGUSR1);
	sigaddset(&set,SIGUSR2);
	pthread_sigmask(SIG_SETMASK,&set,NULL);

	
	pthread_t handler;
	pthread_t *threadinpool;
	pthread_t disp;
	
	//inizializzo la coda di file descriptor
	coda_conn=initcoda();
	if(errno!=0){
		printf("impossibile creare repository\n");
		fflush(stdout);
		return 0;
	}


	int i=0;
	//si itera per 5 volte provando ongi volta il file di configurazione
	while( i<5 && parse(argv[2], &v_configurazione) != 0 ){
		printf("ATTENZIONE: il file di configurazione è considerato non valido, hai la possibilità di scrivere il path di un altro file \n");
		scanf("%s",argv[2]);
		i++;
	}
	//se sono riuscito ad avere un file di configurazione corretto
	if(i<5){

		if (v_configurazione.StatFileName!=NULL){
			ec_null(file_stat=fopen(v_configurazione.StatFileName,"w+"),"impossibile creare file statistiche"); 
		}

		threadinpool = malloc(v_configurazione.ThreadsInPool*(sizeof(pthread_t)));

		repository = icl_hash_create( NB, v_configurazione.StorageSize, v_configurazione.StorageByteSize, v_configurazione.MaxObjSize);

		//creo tutti i thread
		pthread_create(&handler,NULL,sig_handler,NULL);
		pthread_create(&disp, NULL, dispatcher,NULL);
		for(int i=0;i<v_configurazione.ThreadsInPool;i++){

			pthread_create(&threadinpool[i],NULL,worker, NULL);
		}
		


		pthread_join(disp,NULL);
		pthread_join(handler,NULL);

		for(int i=0;i<v_configurazione.ThreadsInPool;i++){
			pthread_join(threadinpool[i],NULL);
		}

		free(threadinpool);

		icl_hash_destroy(repository);
		
		Pthread_mutex_lock(&lk_conn);
				delete_allfd(coda_conn);
		Pthread_mutex_unlock(&lk_conn);
		
		if(file_stat!=NULL && fclose(file_stat)!=0) perror("impossibile chiudere file statistiche");
	}
	
	printf("main termina\n");


    return 0;
}


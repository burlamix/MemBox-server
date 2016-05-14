
#include <message.h>
#include <connections.h>

#include <time.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>
#include <stats.h>
#include <sys/un.h>
#include <string.h>
#define SOCKNAME "sock_server"										//anche qui bisogna definire un modo che le definizioni si facciano da una parte e basta

/**
 * @file  connection.h
 * @brief Contiene le funzioni che implementano il protocollo 
 *        tra i clients ed il server membox
 */

/**
 * @function openConnection
 * @brief Apre una connessione AF_UNIX verso il server membox.
 *
 * @param path Path del socket AF_UNIX 
 * @param ntimes numero massimo di tentativi di retry
 * @param secs tempo di attesa tra due retry consecutive
 *
 * @return il descrittore associato alla connessione in caso di successo
 *         -1 in caso di errore
 */
int openConnection(char* path, unsigned int ntimes, unsigned int secs){

	int fd_c;
	int i=0;
	struct sockaddr_un sa;


	(void) unlink(SOCKNAME);							 
	strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);				/* sistemo l'indirizzo */
	sa.sun_family = AF_UNIX;

	fd_c=socket(AF_UNIX,SOCK_STREAM,0);						/* preparo la socket */						//gestione errori

	while ( (connect( fd_c, (struct sockaddr*) &sa,  sizeof(sa)) == -1) && i < ntimes ) {				//gestione errori
		i++;
		sleep(secs);							 
	}
	printf("connessione del client stabilita\n");	

	if(i==ntimes) return -1;
	else         return fd_c;
}

// -------- server side ----- 

/**
 * @function readHeader
 * @brief Legge l'header del messaggio
 *
 * @param fd     descrittore della connessione
 * @param hdr    puntatore all'header del messaggio da ricevere
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int readHeader(long fd, message_hdr_t *hdr){}

/**
 * @function readData
 * @brief Legge il body del messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore al body del messaggio
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int readData(long fd, message_data_t *data){}


/* da completare da parte dello studente con altri metodi di interfaccia */



// ------- client side ------
/**
 * @function sendRequest
 * @brief Invia un messaggio di richiesta al server membox
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da inviare
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int sendRequest(long fd, message_t *msg){}


/**
 * @function readReply
 * @brief Legge un messaggio di risposta dal server membox
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da ricevere
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int readReply(long fd, message_t *msg){}


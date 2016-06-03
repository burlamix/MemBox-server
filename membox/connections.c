
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

#include <stdlib.h>

#define S_MEX_S 4096
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

						 
	strncpy(sa.sun_path, path ,UNIX_PATH_MAX);				/* sistemo l'indirizzo */
	sa.sun_family = AF_UNIX;

	fd_c=socket(AF_UNIX,SOCK_STREAM,0);						/* preparo la socket */						//gestione errori

	while ( (connect( fd_c, (struct sockaddr*) &sa,  sizeof(sa)) == -1) && i < ntimes ) {				//gestione errori
		i++;
		sleep(secs);							 
	}

	if(i<ntimes)printf("connessione del client stabilita\n");	

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
int readHeader(long fd, message_hdr_t *hdr){
	return (read(fd, hdr ,sizeof(message_hdr_t)));
}

/**
 * @function readData
 * @brief Legge il body del messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore al body del messaggio
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int readData(long fd, message_data_t *data){					// da notare i valori di ritorno chiesti nel header della funzione questa e delle altre

	if (read(fd, &(data->len) ,sizeof(int))==0 ) return 0;

	char* buffer = malloc(data->len);
	char* aus = malloc(S_MEX_S);

	int r;
	int i=0;

	while(i*S_MEX_S < data->len){
		
		r =	read(fd, aus ,S_MEX_S);
		if(r==0) return 0;
		// printf("********%d************read legge %d\n",i,r );

		memcpy(buffer + i*S_MEX_S, aus ,S_MEX_S); 
		i++;
	}

	data->buf = buffer;
	free(aus);
	return 1;
}


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
int sendRequest(long fd, message_t *msg){							//possibilità di migliorare il protocollo mandando messaggi di dimenzione variabile e calcolarsi quanto si è letto e quanto manca da leggere

	write(fd, &msg->hdr ,sizeof(message_hdr_t));

	if(msg->hdr.op == PUT_OP || msg->hdr.op == UPDATE_OP ){
		
		write(fd, &((msg->data).len ),sizeof(int));
		char* aus = malloc(S_MEX_S);
		int i=0,r;

		while(i*S_MEX_S < msg->data.len){

			memcpy(aus, msg->data.buf + i*S_MEX_S, S_MEX_S);
			r=write(fd, aus , S_MEX_S );

			// printf("********%d************write scrive%d\n",i,r );
			i++;
		}
		free(aus);
	}
	return 0;
}


/**
 * @function readReply
 * @brief Legge un messaggio di risposta dal server membox
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da ricevere
 *
 * @return 0 in caso di successo -1 in caso di errore
 */
int readReply(long fd, message_t *msg){
	return 0;
}


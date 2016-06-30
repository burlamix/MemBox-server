
#include <message.h>
#include <connections.h>
#include <err_man.h>


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
int openConnection(char* path, unsigned int ntimes, unsigned int secs) {

	int fd_c;
	int i = 0;
	struct sockaddr_un sa;

	//sistemo l'indirizzo
	strncpy(sa.sun_path, path , UNIX_PATH_MAX);
	sa.sun_family = AF_UNIX;

	//preparo la soket
	ec_meno1_c(fd_c = socket(AF_UNIX, SOCK_STREAM, 0), "socket", return -1);

	while ( (connect( fd_c, (struct sockaddr*) &sa,  sizeof(sa)) == -1) && i < ntimes ) {
		i++;
		sleep(secs);
	}

	if (i < ntimes)printf("connessione del client stabilita\n");
	//se in 10 tentativi non sono riuscito a fare la connect allora ritorno errore
	if (i == ntimes) {perror("connect"); return -1;}
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
 * @return quanto ha letto la read in caso di successo, -1 in caso di errore
 */
int readHeader(long fd, message_hdr_t *hdr) {
	int r = 0;
	ec_meno1_c (r = read(fd, hdr , sizeof(message_hdr_t)), "read", return -1);
	return r;
}

/**
 * @function readData
 * @brief Legge il body del messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore al body del messaggio
 *
 * @return 0 in caso di successo, -1 in caso di errore
 */
int readData(long fd, message_data_t *data) {
	int r = 0;

	//leggo dal fd la lunghezza del messaggio
	ec_meno1_c (r = read(fd, &(data->len), sizeof(int)), "read", return -1);


	ec_null_c (data->buf = malloc(data->len) , "malloc", return -1 );

	char *aus = data->buf;

	int letti = 0;
	int da_leggere = data->len;

	//faccio delle read finche non ho ricevuto la totalità del messaggio
	while (da_leggere > 0) {
		ec_meno1_c( letti = read(fd, aus , da_leggere), "read", free(data->buf); return -1);
		// printf("\nletti----->%d<--\n",letti );
		aus = aus + letti;
		da_leggere = da_leggere - letti;
	}
	return 0;
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
int sendRequest(long fd, message_t *msg) {
	//scrivo l'hdr sul fd
	ec_meno1_c( write(fd, &msg->hdr , sizeof(message_hdr_t)), "wrtie", return -1);

	//nel caso è un messaggio di PUT_OP o UPDATE_OP scrivo anche il body
	if (msg->hdr.op == PUT_OP || msg->hdr.op == UPDATE_OP ) {

		ec_meno1_c( write(fd, &((msg->data).len ), sizeof(int)), "wrtie", return -1);
		char * aus;
		aus = msg->data.buf ;

		int scritti = 0;
		int da_scrivere = msg->data.len;

		//faccio delle write finche non ho ricevuto la totalità del messaggio
		while (da_scrivere > 0) {
			ec_meno1_c( scritti = write(fd, aus , da_scrivere ), "wrtie", return -1);

			// printf("\nscrivo----->%d<--\n",scritti );
			aus = aus + scritti - 1;
			da_scrivere = da_scrivere - scritti;
			// printf("********%d************write scrive%d\n",i,r );
		}
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
int readReply(long fd, message_t *msg) {
	return 0;
}


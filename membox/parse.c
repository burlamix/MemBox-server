#define _GNU_SOURCE	// ca capire bene perchè senza questo la get line non va
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <parse.h>
#include <err_man.h>

/**
 * @function remove_n
 * @brief leva gli \n dalla tringa passata
 *
 * @param s  variabile da modificare
 *
 * @returns la variabile s senza \n
 */
static char * remove_n (char* s) {

	char *p = strchr( s, '\n' );
	if ( p ) *p = 0;
	return s;
}


/**
 * @function assegna_var
 * @brief assegna alla variabile di nome s1 il valore s2, se la variabile di nome s1 non esiste ritorno errore
 *
 * @param s1 nome variabile da aggiornare
 * @param s2 valore variabile da aggiornare
 * @param str_agg struttura di variabili da aggiornare
 *
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
static int assegna_var (char* s1, char* s2, var_conf* str_conf) {

	if (strcmp(s1, "UnixPath" ) == 0)			{strcpy(str_conf->UnixPath, s2); return 0;}
	if (strcmp(s1, "StatFileName") == 0)		{strcpy(str_conf->StatFileName, s2); return 0;}

	if (strcmp(s1, "MaxConnections") == 0)	{str_conf->MaxConnections = atoi(s2); return 0;}
	if (strcmp(s1, "ThreadsInPool") == 0)		{str_conf->ThreadsInPool = atoi(s2); return 0;}
	if (strcmp(s1, "StorageSize") == 0)		{str_conf->StorageSize = atoi(s2); return 0;}
	if (strcmp(s1, "StorageByteSize") == 0)	{str_conf->StorageByteSize = atoi(s2); return 0;}
	if (strcmp(s1, "MaxObjSize") == 0)		{str_conf->MaxObjSize = atoi(s2); return 0;}

	return -1;
}

/**
 * @function parse
 * @brief dal file passato come primo parametro estrae le variabili di configurazione
 *
 * @param path file di consigurazione
 * @param str_agg struttura di variabili da aggiornare
 *
 *
 * @returns 0 in caso di successo,-1 in caso di fallimento
 */
int parse(char* path, var_conf* str_agg)
{
	size_t len = 0;
	char *linea = NULL;
	char *var1 = NULL;
	char *var2 = NULL;

	int  j;

	FILE* fd = NULL;
	ec_null_c(fd = fopen(path, "r"), "fopen", return -1);

	//scorro le righe e scarto quelle vuoto o commentate
	while (getline(&linea, &len, fd) != -1) {

		// ec_meno1_c(getline(&linea, &len, fd),"getline", return -1);
		// a = getline(&linea, &len, fd);
		// printf("\n\n\n\n\n%d->%s<-\n\n\n\n\n",a,linea);

		j = 0;
		while (linea[j] == ' ' || linea[j] == '\t')	{j++;}

		//quando siamo su una riga valida che non è un commento, levo = e i tab e prelevo le due variabili, se non ce ne sono due, ritorno errore
		if (linea[j] != '\n' && linea[j] != '#' && linea[j] != '\0') {

			var1 = strtok (linea, " = \t ");

			if (var1 == NULL) return -1;

			var1 = remove_n(var1);
			var2 = strtok (NULL, " = \t ");

			if (var2 == NULL) return -1;
			var2 = remove_n(var2);

			if (assegna_var(var1, var2, str_agg) == -1) {
				return -1;
			}
		}
	}
	free(linea);
	ec_meno1_c(fclose(fd), "fclose", return -1);

	return 0;
}


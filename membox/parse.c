#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * remove_n (char* s){

	char *p = strchr( s, '\n' );
	if ( p ) *p = 0;
	return s;
}

typedef struct var_conf 
{
	char  UnixPath[100];
	int MaxConnections;
	int ThreadsInPool;
	int StorageSize;
	int StorageByteSize;
	int MaxObjSize;
	char StatFileName[100]; 			//perche se la dichiaro come char * quando poi vado a farci la strcpy mi da errore? creod sia dovuto a un carattere solo come l'ultimo 
}var_conf;

var_conf pippo;

int assegna_var (char* s1, char* s2){

	if(strcmp(s1,"UnixPath" ) == 0)			{strcpy(pippo.UnixPath, s2);return 0;}
	if(strcmp(s1,"StatFileName") == 0)		{strcpy(pippo.StatFileName, s2);return 0;}

	if(strcmp(s1,"MaxConnections") == 0)	{pippo.MaxConnections = atoi(s2);return 0;}
	if(strcmp(s1,"ThreadsInPool") == 0)		{pippo.ThreadsInPool = atoi(s2);return 0;}
	if(strcmp(s1,"StorageSize") == 0)		{pippo.StorageSize = atoi(s2);return 0;}
	if(strcmp(s1,"StorageByteSize") == 0)	{pippo.StorageByteSize = atoi(s2);return 0;}
	if(strcmp(s1,"MaxObjSize") == 0)		{pippo.MaxObjSize = atoi(s2);return 0;}

	return -1;
}


int main()
{
    size_t len = 0;
	char *linea = NULL;
	char *var1 = NULL;
	char *var2 = NULL;

	ssize_t read;
	int i,j;

	FILE* fd = NULL;
	fd = fopen("membox.conf1","r");		//gestione errori

	i=0;
	while(i<15){

		getline(&linea, &len, fd);		//gestione errori

		j=0;
		while(linea[j] == ' ' || linea[j] == '\t')	{j++;}

		if(linea[j]!='\n' && linea[j] != '#'){						//siamo in una riga che dovrebbe essere valida ovvero con un assegnazione di variabile

			var1 = strtok (linea," = \t ");							// non so bene perche se lascio degli spazi dopo una variabile ritorna un valore su var che non è null..
			var1 = remove_n(var1);									// c'è da fare il controllo se c'è più di una stringa
  			var2 = strtok (NULL, " = \t ");
  			var2 = remove_n(var2);
  			// printf("1->%s<-\n", var1);
  			// printf("2->%s<-\n", var2);

			if (assegna_var(var1,var2) == -1) {
				//file di configuarazione non valido come si gestisce?
			}
 			
		}

		i++;
	}

  return 0;
}


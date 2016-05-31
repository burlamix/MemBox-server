#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int parse(char* path,var_conf* str_agg);

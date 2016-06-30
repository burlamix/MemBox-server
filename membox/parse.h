#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct var_conf 
{
	char  UnixPath[255];
	int MaxConnections;
	int ThreadsInPool;
	int StorageSize;
	int StorageByteSize;
	int MaxObjSize;
	char StatFileName[255]; 
}var_conf;

int parse(char* path,var_conf* str_agg);

typedef struct nodo {
    struct nodo *next;
    struct nodo *prec;
    int info;
} nodo;


typedef struct coda_fd{
	int lenght;
	nodo* testa;
	nodo* testa_attesa;
	nodo* coda;
}coda_fd;

//inizializzazione di una coda_fd
coda_fd* initcoda();

//restituisce il puntatore al nodo inserito prima del nodo passato come parametro
nodo* insert_intesta( nodo* n, int c );

//restituisce il puntatore al nodo inserito in coda al nodo passato come parametro
nodo* insert_incoda(nodo*n,int c);

//restituisce il puntatore al nodo successivo
int delete (nodo* n);

//prende in ingresso la testa della lista e cancella tutta la lista
void delete_all(nodo* n);

//cancella il file descpriptor dalla da chiamare con mutua esclusione
void delete_fd(coda_fd* c,nodo * n);

//aggiunge il fd alla coda;
void add_fd( coda_fd* c, int fd);


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

/**
 * @function initcoda
 * @brief alloca e inizializza una struttura coda_fd
 *
 * @return il puntatore alla struttura creata in caso di successo, ritorna NULL e setta errno in caso di errore
 */
coda_fd* initcoda();

/**
 * @function insert_incoda
 * @brief crea un nuovo nodo e lo inserisce dopo il nodo passato come parametro
 *
 * @param n   nodo dopo il quale verrà inserito il nodo appena creato
 * @param c   info contenuta nel nuovo nodo
 *
 * @return il puntatore al nuovo nodo in caso di successo, ritorna NULL e setta ernno in caso di errore
 */
 nodo* insert_incoda(nodo*n,int c);

/**
 * @function delete
 * @brief   eleimina un nodo della coda
 *
 * @param n   puntatore all'elemento da eliminare
 *
 * @return 0 in caso di successo, -1 caso di errore
 */
 int delete (nodo* n);

/**
 * @function delete_coda
 * @brief   elimina una lista doppia
 *
 * @param c  puntatore ad un nodo qualsiasi della lista;
 *
 */
 void delete_coda(nodo* n);

/**
 * @function delete_allfd
 * @brief   elimina una struttura coda_fd
 *
 * @param c  puntatore alla struttura coda_fd che deve essere eliminata
 *
 */
void delete_allfd(coda_fd* c);

/**
 * @function delete_fd
 * @brief   eleimina un elemento da una struttura coda_fd
 *
 * @param c   struttura coda_fd dal quale deve essere eliminato l'elemento
 * @param n   puntatore all'elemento da eliminare
 *
 * @return 0 in caso di successo, -1 caso di errore
 */
int delete_fd(coda_fd* c,nodo * n);

/**
 * @function add_fd
 * @brief   aggiunge un elemento ad una struttura coda_fd
 *
 * @param c   struttura coda_fd al quale deve essere aggiunto un elemento
 * @param fd   contrnuto dell'elemento da aggiungere
 *
 * @return 0 in caso di successo, -1 caso di errore
 */
int add_fd( coda_fd* c, int fd);


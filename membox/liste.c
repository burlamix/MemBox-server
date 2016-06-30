#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err_man.h>
#include "liste.h"

/**
 * @function initcoda
 * @brief alloca e inizializza una struttura coda_fd
 *
 * @return il puntatore alla struttura creata in caso di successo, ritorna NULL e setta errno in caso di errore
 */
coda_fd* initcoda(){
	coda_fd* c;
	ec_null_c( c=malloc(sizeof(coda_fd)),"create coda", return NULL);
	c->lenght=0;
	c->testa_attesa=NULL;
	c->coda=NULL;
	return c;
}


/**
 * @function insert_incoda
 * @brief crea un nuovo nodo e lo inserisce dopo il nodo passato come parametro
 *
 * @param n   nodo dopo il quale verrà inserito il nodo appena creato
 * @param c   info contenuta nel nuovo nodo
 *
 * @return il puntatore al nuovo nodo in caso di successo, ritorna NULL e setta ernno in caso di errore
 */
 nodo* insert_incoda(nodo*n,int c){
	nodo* new;
	ec_null_c(new= malloc(sizeof(nodo)), "create nodo", return NULL );
	new->info=c;
	if(n==NULL){//coda vuota
		new->prec=NULL;
		new->next=NULL;
	}else{
		new->prec=n;
		new->next=n->next;
		n->next=new;
		if(new->next!=NULL) new->next->prec=new;
		
	}
	return new;

}
/**
 * @function delete
 * @brief   eleimina un nodo della coda
 *
 * @param n   puntatore all'elemento da eliminare
 *
 * @return 0 in caso di successo, -1 caso di errore
 */
  int delete (nodo* n){
	if(n!=NULL){
		nodo* aus;
		if(n->prec==NULL && n->next==NULL){//solo un elemento
			free(n);
			return 0;
		}
		if(n->prec!=NULL && n->next!=NULL){//sia il precedente che il successivo
			aus=n->next;
			aus->prec=n->prec;
			n->prec->next=aus;
		}else{
			if(n->prec==NULL)			//solo il successivo
				n->next->prec=NULL;
			else n->prec->next=NULL;	//solo il precedente
		}
		free(n);
		return 0;
	}
	return -1;
}

/**
 * @function delete_coda
 * @brief   elimina una lista doppia
 *
 * @param c  puntatore ad un nodo qualsiasi della lista;
 *
 */
void delete_coda(nodo* n){
	if(n!=NULL){
		delete_coda(n->next);
		delete_coda(n->prec);
		//viene chiuso il fd della comunicazione
		close(n->info);
		free(n);
	}

}

/**
 * @function delete_allfd
 * @brief   elimina una struttura coda_fd
 *
 * @param c  puntatore alla struttura coda_fd che deve essere eliminata
 *
 */
void delete_allfd(coda_fd* c){
		delete_coda(c->testa_attesa);
		free(c);
}

/**
 * @function delete_fd
 * @brief   eleimina un elemento da una struttura coda_fd
 *
 * @param c   struttura coda_fd dal quale deve essere eliminato l'elemento
 * @param n   puntatore all'elemento da eliminare
 *
 * @return 0 in caso di successo, -1 caso di errore
 */
 int delete_fd(coda_fd* c,nodo * n){
	if( n== c->testa_attesa){//devo aggiornare da dove prendo le connessioni
		c->testa_attesa=n->next;
	}
	if(n->next== NULL && c->coda==n){//devo aggiornare dove inserisco le connessioni
		c->coda=n->prec;
	}
	//viene chiuso il filedescriptor della connessione
	ec_meno1_np(close(n->info), return -1);
	ec_meno1_np(delete(n), return -1);
	
	c->lenght--;
	return 0;
}

/**
 * @function add_fd
 * @brief   aggiunge un elemento ad una struttura coda_fd
 *
 * @param c   struttura coda_fd al quale deve essere aggiunto un elemento
 * @param fd   contrnuto dell'elemento da aggiungere
 *
 * @return 0 in caso di successo, -1 caso di errore
 */
int add_fd( coda_fd* c, int fd){

	ec_null_np(c->coda = insert_incoda(c->coda,fd), return -1);
	c->lenght++;
	if(c->testa_attesa==NULL){
			c->testa_attesa= c->coda;
	}
	return 0;
}
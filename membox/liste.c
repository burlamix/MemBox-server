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
	c->testa=NULL;
	c->testa_attesa=NULL;
	c->coda=NULL;
	return c;
}

/**
 * @function insert_intesta
 * @brief crea un nuovo nodo e lo inserisce prima del nodo passato come parametro
 *
 * @param n   nodo prima del quale verrà inserito il nodo appena creato
 * @param c   info contenuta nel nuovo nodo
 *
 * @return il puntatore al nuovo nodo in caso di successo, ritorna NULL e setta ernno in caso di errore
 */
nodo* insert_intesta( nodo* n, int c ){
	nodo* new;
	ec_null_c(new= malloc(sizeof(nodo)), "create nodo", return NULL );
	new->info=c;
	if(n==NULL){
		new->next=NULL;
		new->prec=NULL;
	}else{
		new->prec=n->prec;
		new->next=n;
		n->prec=new;
	}
	return new;
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
	if(n==NULL){
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
		if(n->prec==NULL && n->next==NULL){
			free(n);
			return 0;
		}
		if(n->prec!=NULL && n->next!=NULL){
			aus=n->next;
			aus->prec=n->prec;
			n->prec->next=aus;
		}else{
			if(n->prec==NULL)
				n->next->prec=NULL;
			else n->prec->next=NULL;
		}
		fflush(stdout);
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
	if( n== c->testa_attesa){
		c->testa_attesa=n->next;
	}
	if(n->next== NULL && c->coda==n){
		c->coda=n->prec;
	}
	close(n->info);
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
	if(c->testa==NULL){
		c->testa=c->coda;

	}
	if(c->testa_attesa==NULL){
			c->testa_attesa= c->coda;
	}
	return 0;
}




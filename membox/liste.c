#include <stdlib.h>
#include <stdio.h>
#include "liste.h"


coda_fd* initcoda(){
	coda_fd* c;
	if(( c=malloc(sizeof(coda_fd))) == NULL){
		perror("create coda");
	}else{
		c->lenght=0;
		c->testa=NULL;
		c->testa_attesa=NULL;
		c->coda=NULL;
	}
	return c;
}

//restituisce il puntatore al nodo inserito prima del nodo passato come parametro
nodo* insert_intesta( nodo* n, int c ){
	nodo* new= malloc(sizeof(nodo));
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
//restituisce il puntatore al nodo inserito in coda al nodo passato come parametro
nodo* insert_incoda(nodo*n,int c){
	nodo* new= malloc (sizeof(nodo));
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

//restituisce il puntatore al nodo successivo
int delete (nodo* n){
	if(n!=NULL){
		nodo* aus;
		if(n->prec==NULL && n->next==NULL){
			//free(n);
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
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nBOOOOM\n");
		fflush(stdout);
		//free(n);
		return 0;
	}
	return -1;
}

//prende in ingresso la testa della lista e cancella tutta la lista
void delete_all(nodo* n){

	if(n!=NULL){
		nodo* aus=n->next;
		n->next=NULL;
		//free(n);
		delete_all(aus);
	}

}

//cancella il file descpriptor dalla da chiamare con mutua esclusione
void delete_fd(coda_fd* c,nodo * n){
	if( n== c->testa_attesa){
		c->testa_attesa=n->next;
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nBOOOOM\n");
	}
	if(n->next== NULL && c->coda==n){
		c->coda=n->prec;
	}
	delete(n);
	
	c->lenght--;
}

void add_fd( coda_fd* c, int fd){

	c->coda = insert_incoda(c->coda,fd);
	c->lenght++;
	if(c->testa==NULL){
		c->testa=c->coda;

	}
	if(c->testa_attesa==NULL){
			c->testa_attesa= c->coda;
	}
	
}




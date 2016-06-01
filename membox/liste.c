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
nodo* delete (nodo* n){
	if(n!=NULL){
		nodo* aus;
		if(n->prec!=NULL && n->next!=NULL){
			aus=n->next;
			aus->prec=n->prec;
			n->prec->next=aus;
		}else{
			if(n->prec==NULL)
				aus=n->next;
			else aus=NULL;
		}
		free(n);
		return aus;
	}else return NULL;
}

//prende in ingresso la testa della lista e cancella tutta la lista
void delete_all(nodo* n){

	if(n!=NULL){
		nodo* aus=n->next;
		n->next=NULL;
		free(n);
		delete_all(aus);
	}

}

//cancella il file descpriptor dalla da chiamare con mutua esclusione
void delete_fd(coda_fd* c,nodo * n){
	if(n!=NULL && n->prec ==NULL){
		c->testa=delete(n);
	}else{
		n->prec->next=delete(n);
	}
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
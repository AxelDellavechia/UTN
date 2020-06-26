#include <iostream>
#include "Funciones.h"


using std::cout;
using std::cin;
using std::endl;

void ImprimirBanner(){
	cout << "\t+-----------------------------------------------------+" << endl;
	cout << "\t|Algoritmos y Estructuras de Datos                    |" << endl;
	cout << "\t|UTN-FRBA-Curso K1051                                 |" << endl;
	cout << "\t|TP No 17 - Stacks y Queues mediante listas enlazadas |" << endl;
	cout << "\t+-----------------------------------------------------+\n\n" << endl;
};

Stack& Push (Stack& s, int& x){
	Nodo* n = new Nodo(); 	
	n->val = x;			
	n->next = s.top;	
	s.top = n;				
	return s;
};

Stack& Pop (Stack& s, int& x){
	Nodo* indice;			
	if (s.top != nullptr){		
		indice = s.top;			
		x = indice->val;		
		s.top = indice->next;	
		delete indice;			
								
	}
	return s;
};

Queue& Enqueue (Queue& q, int& x){
	Nodo* n = new Nodo();
	n->val = x;
	
	if (q.rear == nullptr){
		q.front = n;
		q.rear = n;
	} else {
		(q.rear)->next = n;
		q.rear = n;
	}
	
	return q;
};

Queue& Dequeue (Queue& q, int& x){
	Nodo* indice;
	
	if (q.rear != nullptr){
		indice = q.front;
		x = indice->val;
		q.front = indice->next;
		delete indice;	
	}
	
	return q;
};

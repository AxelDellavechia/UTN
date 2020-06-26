#include <iostream>
#include "Funciones.h"

using std::cout;
using std::cin;
using std::endl;
char resp;

int main () {
		
		ImprimirBanner();
		Stack Pila ;
		Queue Cola ;
		MostrarMenu (Pila,Cola);
	}

void ImprimirBanner(){
	cout << "\t+-------------------------------------+" << endl;
	cout << "\t|Algoritmos y Estructuras de Datos    |" << endl;
	cout << "\t|UTN-FRBA-Curso K1051                 |" << endl;
	cout << "\t|TP No 16 - Stacks y Queues           |" << endl;
	cout << "\t+-------------------------------------+\n\n" << endl;
};

int MostrarMenu(Stack& s, Queue& q){
		int opcionMenu;
		int entero;
Menu:	cout << endl << endl;
        cout << "Seleccione opcion:\n"
             << "1:Hacer push de un entero en el stack\n"
             << "2:Hacer pop del stack\n"
             << "3:Hacer pop del stack hasta vaciarlo (composicion de pops recursivos)\n"
             << "4:Encolar un entero en la cola\n"
             << "5:Desencolar de la cola\n"
             << "6:Mostrar stack y cola\n"
             << "0:Salir\n"
        	 << endl ;
        cin >> opcionMenu;	
        
  
		switch(opcionMenu){
            
           case 1:
           		do {
           			if (s.largo < s.sec.size() ){
	           			cout << endl << endl << "Ingrese el entero que ingresar en el push: ";
		            	cin >> entero;
		            	Push (s, entero);
					} 
					else {
						cout << endl << endl << "La Pila esta llena.";
						resp = 'N';
						break;
					}
					cout << "Desea Agregar otro elemento a la Pila, responda con S o s para positivo o cualquier otro caracter para volver al menu. Su respuesta es: "	;
					cin >> resp;
           		}
				while (resp == 'S' || resp == 's');
				goto Menu;
				break;
            case 2:
            	Pop(s, entero);
                goto Menu;
				break;
            case 3:
            	PopRecursivo(s, entero);
                goto Menu;
				break;
            case 4:
            	    do {
		           			if (q.largo < s.sec.size() ){ 
		           			cout << endl << endl << "Ingrese el entero que encolar: ";
	            			cin >> entero;
			            	Enqueue (q, entero);
					} 
					else {
						cout << endl << endl << "La Cola esta llena.";
						resp = 'N';
						break;
					}
					cout << "Desea Agregar otro elemento a la cola, responda con S o s para positivo o cualquier otro caracter para volver al menu. Su respuesta es: "	;
					cin >> resp;
           		}
				while (resp == 'S' || resp == 's');
                goto Menu;
				break;
            case 5:
            	Dequeue (q, entero);
                goto Menu;
				break;
            case 6:
            	cout << endl << "El contenido del Stack es:" << endl;
            	for (int i = 0; i < s.sec.size(); ++i)
            		cout << s.sec.at(i) << " ";
            	cout << endl << endl << "El contenido de la Cola es:" << endl;
            	for (int i = 0; i < q.sec.size(); ++i)
            		cout << q.sec.at(i) << " ";
                goto Menu;
				break;
            default:
            	return -1;
        }
        
        return 0;
};

Stack& Push (Stack& stck, int& ent){
		stck.sec.at(stck.largo) = ent;
		++stck.largo;
		cout << endl << "La altura del Stack es: " << stck.largo << endl;	
		
	return stck;
};

Stack& Pop (Stack& stck, int& ent){
	if (stck.largo > 0){
		ent = stck.sec.at(stck.largo - 1);
		cout << endl << endl << "El pop da como resultado el entero: " << ent;
		--stck.largo;
	}
	else
		cout << endl << endl << "El Stack esta vacio.";
	
	cout << endl << "La altura del Stack es: " << stck.largo;	
	return stck;
};

Stack& PopRecursivo (Stack& stck, int& ent){
	if (stck.largo == 0){
		cout << endl << endl << "El Stack esta vacio.";
		cout << endl << "La altura del Stack es: " << stck.largo;
	}
			
	while (stck.largo > 0){
		ent = stck.sec.at(stck.largo - 1);
		cout << endl << endl << "El pop da como resultado el entero: " << ent;
		--stck.largo;
		PopRecursivo (stck, ent);
	}

	return stck;
};

Queue& Enqueue(Queue& qiu, int& ent){
		qiu.sec.at((qiu.front + qiu.largo) % 10) = ent;
		++qiu.largo;
		cout << endl << "El largo de la Cola es: " << qiu.largo << endl;
		
	return qiu;
};

Queue& Dequeue(Queue& qiu, int& ent){
	if (qiu.largo > 0){
		ent = qiu.sec.at(qiu.front);
		cout << endl << endl << "El desencolar da como resultado el entero: " << ent;
		qiu.front = ((++qiu.front) % 10);
		--qiu.largo;
	}
	else
		cout << endl << endl << "La Cola esta vacia.";
	
	cout << endl << "La posicion del front es: " << qiu.front;
	
	return qiu;
};

				

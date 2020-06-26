//  AED - TP17 -  TP17.cpp
//  TP17 - Stacks y Queues mediante listas enlazadas (punteros)
//  Creado por Grupo 2 el 29/10/15.

#include <iostream>
#include "Funciones.h"
#include "Structs.h"

using std::cin;
using std::cout;
using std::endl;

int main (){
	int x, y, z;
	
	ImprimirBanner();
	
	Stack st;
	Queue qu;
	

	cout << "A continuacion ingresa 3 valores que se sumaran a la PILA" << endl ;
	cin >> x >> y >> z;
	Push (Push (Push (st, x), y), z);


	x = 0;
	y = 0;
	z = 0;
		
	Pop (Pop (Pop (st, x), y), z);
	cout << endl  << "Los valores obtenidos del POP de su PILA son: " << endl << x << endl << y << endl << z << endl << endl;
	

	cout << "A continuacion ingresa 3 valores que se sumaran a la COLA" << endl ;
	cin >> x >> y >> z;
	Enqueue (Enqueue (Enqueue (qu, x), y), z);
	

	x = 0;
	y = 0;
	z = 0;
	
	Dequeue (Dequeue (Dequeue (qu, x), y), z);
	cout << endl  << "Los valores que se quitaron de la COLA son: " << endl << x << endl << y << endl << z << endl;
}

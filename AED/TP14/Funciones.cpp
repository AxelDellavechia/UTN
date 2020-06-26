#include "Funciones.h"
#include <iostream>

using namespace std;

void DisplayArray(const array<int, 10>& ArrayAMostrar){
	cout << endl;
	for (int i = 0; i < ArrayAMostrar.size(); i++)
		cout << ArrayAMostrar.at(i) << endl;
}

void OrdenarArrayPorSeleccion(array<int, 10> ArrayAOrdenar){
	int cursor;
	int i, j;
	int memoria;
	cout << endl << "Ordenando por seleccion..." << endl;
	
	for (i = 0; i < (ArrayAOrdenar.size() - 1); i++){
		cursor = i;
		for (j = i+1; j < ArrayAOrdenar.size(); j++){
			if (ArrayAOrdenar.at(j) < ArrayAOrdenar.at(cursor)){
				cursor = j;
			}

		}
	// Funcion swap	
	memoria = ArrayAOrdenar.at(i);
	ArrayAOrdenar.at(i) = ArrayAOrdenar.at(cursor);
	ArrayAOrdenar.at(cursor) = memoria;
	}
	
	DisplayArray(ArrayAOrdenar);
	IsArrayOrdenadoEnOrdenNoDecreciente(ArrayAOrdenar);
}

void OrdenarArrayPorInsercion(array<int, 10> ArrayAOrdenar){
	int cursor = 0;
	int memoria = 0;
	cout << endl << "Ordenando por insercion..." << endl;
	
	for (int i = 1; i < ArrayAOrdenar.size(); i++){
		for (cursor = i; ((cursor > 0) && (ArrayAOrdenar.at(cursor) < ArrayAOrdenar.at(cursor - 1))); cursor--){
			memoria = ArrayAOrdenar.at(cursor-1);
			ArrayAOrdenar.at(cursor-1) = ArrayAOrdenar.at(cursor);
			ArrayAOrdenar.at(cursor) = memoria;
		}
	}
	
	DisplayArray(ArrayAOrdenar);
	IsArrayOrdenadoEnOrdenNoDecreciente(ArrayAOrdenar);
}

void OrdenarArrayPorBurbujeo(array<int,10> ArrayAOrdenar){
	int aux;
	cout << endl << "Ordenando por burbujeo..." << endl;
	
	for(int i = 0; i < ArrayAOrdenar.size(); i++){
		for(int j = 0; j < ArrayAOrdenar.size() - 1; j++){
			if(ArrayAOrdenar.at(j) > ArrayAOrdenar.at(j+1)){
				aux = ArrayAOrdenar.at(j);
				ArrayAOrdenar.at(j) = ArrayAOrdenar.at(j+1);
				ArrayAOrdenar.at(j+1) = aux; 
			}
		}
	}
	
	DisplayArray(ArrayAOrdenar);
	IsArrayOrdenadoEnOrdenNoDecreciente(ArrayAOrdenar);
}

int Particionar(array<int,10>& ArrayAOrdenar, int limIzquierdo, int limDerecho){
	int x = ArrayAOrdenar.at(limIzquierdo);
	int i = limIzquierdo;
	int j;
	int memoria;
	
	for (j = limIzquierdo + 1; j < limDerecho; j++){
		if (ArrayAOrdenar.at(j) <= x){
			i++;
		//	Funcion swap
			memoria = ArrayAOrdenar.at(i);
			ArrayAOrdenar.at(i) = ArrayAOrdenar.at(j);
			ArrayAOrdenar.at(j) = memoria;
		}
	}
	// Funcion swap
	memoria = ArrayAOrdenar.at(i);
	ArrayAOrdenar.at(i) = ArrayAOrdenar.at(limIzquierdo);
	ArrayAOrdenar.at(limIzquierdo) = memoria;

	return i;
}

void OrdenarArrayPorQSort(array<int,10>& ArrayAOrdenar, int limIzquierdo, int limDerecho){
	int pivot;
	
	if (limIzquierdo < limDerecho){
		pivot = Particionar(ArrayAOrdenar, limIzquierdo, limDerecho);
		OrdenarArrayPorQSort(ArrayAOrdenar, limIzquierdo, pivot);
		OrdenarArrayPorQSort(ArrayAOrdenar, pivot+1, limDerecho);
	}
}

bool IsArrayOrdenadoEnOrdenNoDecreciente (const array<int, 10>& ArrayAEvaluar){
	for (int i = 1; i < ArrayAEvaluar.size(); i++){
		if (ArrayAEvaluar.at(i) < ArrayAEvaluar.at(i - 1)){
			cout << endl << "El array no esta ordenado en orden no decreciente." << endl;
			return false;
		}
	}
	cout << endl << "El array esta correctamente ordenado en orden no decreciente." << endl;
	return true;
}

#include <iostream>
#include <array>
#include "Funciones.h"

using namespace std;

array <int, 10> NumerosOrden = {21, 6, 15, 7, 99, 12, 24, 2, 33, 13};

int main (){
	OrdenarArrayPorSeleccion(NumerosOrden);
		
	OrdenarArrayPorInsercion(NumerosOrden);
		
	OrdenarArrayPorBurbujeo(NumerosOrden);
	
	cout << endl << "Ordenando por Quick Sort..." << endl;
	OrdenarArrayPorQSort(NumerosOrden, 0, 10);
	
	DisplayArray(NumerosOrden);
	
	IsArrayOrdenadoEnOrdenNoDecreciente(NumerosOrden);
}

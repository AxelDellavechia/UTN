#include <iostream>
#include <array>
#include "Funciones.h"

using namespace std;

array <int, 10> NumerosOrden = {21, 6, 15, 7, 99, 12, 24, 2, 33, 13};

int main (){	
	int limIzq = 0;
	int limDer = 10;
	
	cout << "El array original: ";
	
	DisplayArray(NumerosOrden);
	
	BusquedaSecuencial(NumerosOrden);
	
	OrdenarArrayPorInsercion(NumerosOrden);
		
	DisplayArray(NumerosOrden);
	
	BusquedaBinaria(NumerosOrden, limIzq, limDer);	
}

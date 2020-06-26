#include "Funciones.h"
#include <iostream>

using namespace std;

void DisplayArray(const array<int, 10> &ArrayAMostrar){
	cout << endl;
	for (int i = 0; i < ArrayAMostrar.size(); i++)
		cout << ArrayAMostrar.at(i) << " ";
	cout << endl;
	if (IsArrayOrdenadoEnOrdenNoDecreciente(ArrayAMostrar))
		cout << "El array esta correctamente ordenado en orden no decreciente." << endl;
	else
		cout << "El array no esta ordenado en orden no decreciente." << endl << endl;
}

bool IsArrayOrdenadoEnOrdenNoDecreciente (const array<int, 10> &ArrayAEvaluar){
	for (int i = 1; i < ArrayAEvaluar.size(); i++){
		if (ArrayAEvaluar.at(i) < ArrayAEvaluar.at(i - 1))
			return false;
	}
	return true;
}

void BusquedaSecuencial(array<int, 10> &elArray){
      cout << "Se buscara el numero 7 en el array utilizando busqueda sencuencial:" << endl;
      int flag = 0;    //flag off
      int i = 0;

      for(i=0; elArray.size(); i++){
	    if (elArray.at(i) == 7) {
        	flag = 1;  	//flag on
            break ;    
        }
      }
      
      if (flag)    
        cout << "El numero buscado se encuentra en la posicion " << i <<" (siendo 0 la primera posicion).\n";
      
      else
        cout << "El numero no se ha encontrado en el array." << endl << endl;           
}

void OrdenarArrayPorInsercion(array<int, 10> &ArrayAOrdenar){
	int cursor = 0;
	int memoria = 0;
	
	cout << endl << "Aplicando metodo de ordenamiento por insercion: ";
	
	for (int i = 1; i < ArrayAOrdenar.size(); i++){
		for (cursor = i; ((cursor > 0) && (ArrayAOrdenar.at(cursor) < ArrayAOrdenar.at(cursor - 1))); cursor--){
			memoria = ArrayAOrdenar.at(cursor-1);
			ArrayAOrdenar.at(cursor-1) = ArrayAOrdenar.at(cursor);
			ArrayAOrdenar.at(cursor) = memoria;
		}
	}
}

void BusquedaBinaria(array<int,10> &elArray, int limIzquierdo, int limDerecho){
	
    int pos = (limIzquierdo + limDerecho) / 2;
    int clave = 7;
    cout << "Se buscara el numero 7 en el array utilizando busqueda binaria:" << endl;

    while((elArray.at(pos) != clave) && (limIzquierdo <= limDerecho)){
        if (elArray.at(pos) > clave)            
            limDerecho = pos - 1;    
        else                                                
            limIzquierdo = pos + 1;     
             
        pos = (limIzquierdo + limDerecho) / 2;
    }
       
    if (limIzquierdo <= limDerecho)
    	cout << "El numero buscado se encuentra en la posicion " << pos <<" (siendo 0 la primera posicion).\n" << endl; 
    else
        cout << "El numero no se ha encontrado en el array." << endl << endl;
}


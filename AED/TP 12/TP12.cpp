#include <iostream>
#include "Circulo.h"
#include "Rectangulo.h"


using namespace std;

enum Menu {
	MayorArea,
	PerimetroMayorAlPromedio,
	CirculosDentroRectangulo,
	SALIR = -1
};

bool SeguirLeyendo(){
	char tecla;
	cin >> tecla;
	if ((tecla == 's') || (tecla == 'S'))
		return true;
	else
		return false;
}

int main() {

regreso: cout 	<< "Indique que accion desea realizar a continuaci\xA2n:" << endl << endl 
				<< "1 - Obtener el circulo de mayor \xA0rea." << endl 
				<< "2 - Obtener los circulos que tienen circunferencia mayor al promedio." << endl 
				<< "3 - Mostrar los rect\xA0ngulos que caben en un circulo." << endl 
				<< "0 - Salir del programa" << endl << endl;
	
	int respuestamenu;
	cin >> respuestamenu;
	
	switch (respuestamenu - 1){
		
		case MayorArea:
		{
			GetCirculoMayorArea();
			break;
		}
		
		case PerimetroMayorAlPromedio:
		{
			GetCirculosPerimetroSuperiorPromedio();
			break;
		}		
		
		case CirculosDentroRectangulo:
			RectangulosCirculos();
			break;
		
		case SALIR:
		return 0;
		break;
		
		default:
			cout << "Ingreso una opci\xA2n no valida, se volver\xA0 a mostrar el men\xA3 de selecci\xA2n.";
			goto regreso;
			break;
		}
};

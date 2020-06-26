#include <iostream>
#include "Circulo.h"
#include <array>
#include <cmath>
#include <iostream>
#include <string>

using namespace std;

const static double Pi = atan(1.0) * 4;

void LeerCirculo(Circulo &c) {
	
	cout << endl << endl 	<< "Estimado usuario, deber\xa0 ingresar como m\xa0ximo 100 circulos." << endl
							<< "Ingrese los siguientes datos: " << endl << endl 
	<< "Radio del circulo: " ;
	cin >> c.radio; 
	cout << endl <<"Coordenada X del centro del circulo: " ;
	cin >> c.centro.posx ;
	cout << endl << "Coordenada Y del centro del circulo: " ;
	cin >> c.centro.posy;
};

double GetArea(const Circulo &c) {
	return Pi * (c.radio * c.radio);
};

double GetPerimetro(const Circulo &c) {
	return 2 * Pi * c.radio ;
};


void GetCirculosPerimetroSuperiorPromedio() {
	
	Circulo c;
	array < Circulo, 100 > VariosCirculos;
	
	double PerimetroTotal = 0;
	int i = 0;
	
	do{
		LeerCirculo(c);
		VariosCirculos[i] = c;
		PerimetroTotal += GetPerimetro(VariosCirculos[i]);
		cout 	<< endl << "Desea continuar ingresando circulos?" << endl 
			<< "En caso afirmativo, ingrese \'S\', en caso negativo, cualquier otro caracter. ";
		i++;
	} while (SeguirLeyendo());
	
	PerimetroTotal = (PerimetroTotal / i);
	cout<< endl << endl << "Estos son los circulos con mayor circunferencia al promedio:" << endl << endl ;
		for (int j = 0; j < i; j++){
			if (GetPerimetro(VariosCirculos[j]) > PerimetroTotal) 
				cout << "El circulo de posici\xA2n " << j << " tiene una circunferencia mayor al promedio ( " << PerimetroTotal << " )"<< endl << endl;
		}
}

void GetCirculoMayorArea(){
	
	Circulo c;
	array < Circulo, 100 > VariosCirculos;
	
	double Area = 0;
	double AreaMayor = 0;
	int i = 0;
	int j = 0;
	
	do{	// Ingresa los circulos al array de circulos
		LeerCirculo(c);	
		VariosCirculos[i] = c;
		if (i == 0) 
			AreaMayor = GetArea(VariosCirculos[0]);
		
		else {
			Area = GetArea(VariosCirculos[i]);
			if ( Area > AreaMayor ) {
				AreaMayor = Area;	
				j = i ;}
		}
	cout 	<< endl << "Desea continuar ingresando circulos?" << endl 
		<< "En caso afirmativo, ingrese \'S\', en caso negativo, cualquier otro caracter. ";	
	i++;
	} while (SeguirLeyendo());
	cout << endl << endl << "El circulo de mayor \xA0rea es el que tiene radio ( " << VariosCirculos[j].radio << " ) y centro ( " << VariosCirculos[j].centro.posx << " , " << VariosCirculos[j].centro.posy << " )";
}

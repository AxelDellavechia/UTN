#include "Rectangulo.h"
#include "Circulo.h"
#include <iostream>
#include <cmath>
#include <array>

using namespace std; 

void LeerRectangulo(Rectangulo &r){
	cout << endl << endl << "Estimado usuario, deber\xa0 ingresar como m\xa0ximo 100 rect\xa0ngulos." << endl
	<< "Ingrese los siguientes datos: " << endl << endl 
	<< "Coordenada X del vertice A de su rect\xa0ngulo: " ;
	cin >> r.verticeA.posx; 
	cout << endl <<"Coordenada Y del vertice A del rect\xa0ngulo: " ;
	cin >> r.verticeA.posy ;
	cout << endl << "Coordenada X del vertice B del rect\xa0ngulo: ";
	cin >> r.verticeB.posx ;
	cout << endl << "Coordenada Y del vertice B del rect\xa0ngulo: ";
	cin >> r.verticeB.posx ;
	
	cout << endl << "Desea continuar ingresando rect\xa0ngulos?" << endl 
	<< "En caso afirmativo, ingrese \'S\', en caso negativo, cualquier otro caracter. ";
};

Punto2D GetVerticeA (const Rectangulo &r){
	return r.verticeA;
};

Punto2D GetVerticeB (const Rectangulo &r){
	return r.verticeB;
};

Punto2D GetVerticeC (const Rectangulo &r){
	Punto2D VerticeC;
	VerticeC.posx = r.verticeA.posx ;
	VerticeC.posy = r.verticeB.posy ;
	return VerticeC;
}

Punto2D GetVerticeD (const Rectangulo &r) {
	Punto2D VerticeD;
	VerticeD.posx = r.verticeB.posx ;
	VerticeD.posy = r.verticeA.posy ;
	return VerticeD;
}


double GetDistancia (const Punto2D &a, const Punto2D &b) {
	return sqrt((a.posx - b.posx)*(a.posx - b.posx)+(a.posy - b.posy)*(a.posy - b.posy));
}

bool PuntoMenorQueElRadio (const Punto2D &verticeRect, const Circulo &c ){
	return GetDistancia (c.centro, verticeRect) <= c.radio;
}

bool EntraEnCirculo(Rectangulo r, Circulo c) {
	return (PuntoMenorQueElRadio (GetVerticeA(r),c) && PuntoMenorQueElRadio (GetVerticeB(r),c) && PuntoMenorQueElRadio (GetVerticeC(r),c) && PuntoMenorQueElRadio (GetVerticeD(r),c) );
};

void RectangulosCirculos () {
	
	Circulo c;
	Rectangulo r;
	
	array < Circulo, 100 > VariosCirculos;
	array < Rectangulo, 100 > VariosRectangulos;
	
	int h = 0;
	int i = 0;
	
	do {
		LeerCirculo(c);
		VariosCirculos[h] = c;
		cout 	<< endl << "Desea continuar ingresando circulos?" << endl 
				<< "En caso afirmativo, ingrese \'S\', en caso negativo, cualquier otro caracter. ";
		h++;		
	} while (SeguirLeyendo());
	
	cin.clear();
	
	do {
		LeerRectangulo(r);
		VariosRectangulos[i] = r;
		cout 	<< endl << "Desea continuar ingresando rectangulos?" << endl
				<< "En caso afirmativo, ingrese \'S\', en caso negativo, cualquier otro caracter. ";
		i++;		
	} while (SeguirLeyendo());
	
	for (int k=0; k < h; k++) {
		for (int j=0; j < i; j++) {
			if (EntraEnCirculo(VariosRectangulos[j], VariosCirculos[k]))
				cout << endl << "El rectangulo de posici\xa2n " << j << " entra en el circulo de posici\xa2n " << k << " ." << endl;
		}
	}
}

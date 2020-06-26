#include "Punto2D.h"

Punto2D p;

istream& LeerPunto2D(Punto2D &p) 
{
	return cin >> p.posx >> p.posy;
};

void MostrarPunto2D (Punto2D &p){
	cout << "" << endl << p.posx  << ""  << endl << p.posy ;
};

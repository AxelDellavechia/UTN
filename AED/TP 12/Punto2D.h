#ifndef PTO2D_H
#define PTO2D_H

#include <iostream>
using namespace std; //Recordar usar el namespace std o si no usar std::istream

struct Punto2D {
	double posx;	// proyeccion del punto sobre el eje x
	double posy;	// proyeccion del punto sobre el eje y
};

istream& LeerPunto2D(Punto2D &p);

void MostrarPunto2D (Punto2D &mp);

#endif

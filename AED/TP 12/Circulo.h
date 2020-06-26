#ifndef CIRCULO_H 
#define CIRCULO_H

#include <iostream>
#include "Punto2d.h" 

using namespace std; 

struct Circulo {
	double radio;
	Punto2D centro;
};

bool SeguirLeyendo();

void LeerCirculo(Circulo &c);

double GetArea(const Circulo &c);

double GetPerimetro(const Circulo &c);

void GetCirculosPerimetroSuperiorPromedio();

void GetCirculoMayorArea();

#endif

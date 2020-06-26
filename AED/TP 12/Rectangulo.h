#ifndef RECTANGULO_H
#define RECTANGULO_H

#include <iostream>
#include "Punto2d.h"
#include "Circulo.h" 

using namespace std;

struct Rectangulo {
	Punto2D verticeA;
	Punto2D verticeB;
};

bool SeguirLeyendo();

void LeerRectangulo(Rectangulo &r);

bool EntraEnCirculo(const Rectangulo r, const Circulo c);

Punto2D GetVerticeA (const Rectangulo &r);

Punto2D GetVerticeB (const Rectangulo &r);

Punto2D GetVerticeC (const Rectangulo &r);

Punto2D GetVerticeD (const Rectangulo &r);

double GetDistancia (const Punto2D &a, const Punto2D &b);

bool PuntoMenorQueElRadio (const Punto2D &p, const Circulo &c );

void RectangulosCirculos();

#endif

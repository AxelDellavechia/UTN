#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <array>
using std::array;

void DisplayArray(const array<int, 10> &ArrayAMostrar);
void OrdenarArrayPorInsercion(array<int, 10> &ArrayAOrdenar);
void BusquedaSecuencial(array<int, 10> &elArray);
void BusquedaBinaria(array<int,10> &elArray, int limIzquierdo, int limDerecho);
bool IsArrayOrdenadoEnOrdenNoDecreciente (const array<int, 10> &ArrayAEvaluar);

#endif

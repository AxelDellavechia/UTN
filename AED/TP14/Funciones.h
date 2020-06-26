#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <array>
using std::array;

void DisplayArray(const array<int, 10>& ArrayAMostrar);
void OrdenarArrayPorSeleccion(array<int, 10> ArrayAOrdenar);
void OrdenarArrayPorInsercion(array<int, 10> ArrayAOrdenar);
void OrdenarArrayPorBurbujeo(array<int, 10> ArrayAOrdenar);
void OrdenarArrayPorQSort(array<int,10>& ArrayAOrdenar, int limIzquierdo, int limDerecho);
bool IsArrayOrdenadoEnOrdenNoDecreciente (const array<int, 10>& ArrayAEvaluar);

#endif

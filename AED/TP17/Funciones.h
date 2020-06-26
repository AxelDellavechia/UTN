
#ifndef FUNCIONES_H
#define FUNCIONES_H

#include "Structs.h"

void ImprimirBanner();
//int MostrarMenu(Stack&, Queue&);

Stack& Push (Stack&, int&);
Stack& Pop (Stack&, int&);

Queue& Enqueue(Queue&, int&);
Queue& Dequeue(Queue&, int&);

#endif

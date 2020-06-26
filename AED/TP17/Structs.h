#ifndef STRUCTS_H
#define STRUCTS_H

struct Nodo {
	int val;
	Nodo* next;	
};

struct Stack {
	struct Nodo* top = nullptr;
};

struct Queue {
	struct Nodo* front  = nullptr;
	struct Nodo* rear  = nullptr;
};

#endif

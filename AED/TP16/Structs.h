
#ifndef STRUCTS_H
#define STRUCTS_H

#include <array>

using std::array;

struct Stack {
	array <int, 10> sec;
	unsigned int largo = 0;
};

struct Queue {
	array <int, 10> sec;
	unsigned int front = 0;
	unsigned int largo = 0;
};

#endif

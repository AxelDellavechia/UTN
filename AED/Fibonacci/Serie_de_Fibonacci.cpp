//  Serie_de_Fibonacci.cpp
//  Autor: Federico Bruzone
//  Creado el 19/6/15.

#include <iostream>

int main() {
	using std::cout;
	using std::cin;
	using std::endl;
		
	cout << "Bienvenido al graficador de series de Fibonacci.\n"
		<< "Inserte la cantidad de numeros de la serie de Fibonacci que desea ver: ";
	unsigned int limite = 0; //cantidad de elementos de la serie de Fibonacci a mostrar 
	cin >> limite;
	cout << endl << "Estos son los primeros " << limite << " numeros de la serie de Fibonacci:" << endl;
	
	unsigned int a = 0;
	unsigned int b = 0;
	unsigned int x = 0;
		
	for (unsigned i = 0; i <= limite; i++){ //itera desde i=0 hasta la cantidad de elementos de la serie a mostrar
		
		if (i == 1){ //Si n=1, asigna 1 a la variable x.
			x = 1;
		}
		cout << x << " ";
		a = b;
		b = x;
		x = a + b;
	}
	// Primera iteracion: imprime 0; a = 0; b = 0; x = a+b = 0 
	// Segunda iteracion: x = 1; imprime 1; a = 0; b = 1; x = a+b = 1
	// Tercera iteracion: imprime 1; a = 1; b = 1; x = a+b = 2
	// Cuarta iteracion: imprime 2; a = 1; b = 2; x = a+b = 3
	// Quinta iteracion: imprime 3; a = 2; b = 3; x = a+b = 5
	// etc.
}

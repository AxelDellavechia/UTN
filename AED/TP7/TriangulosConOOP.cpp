/* Federico Bruzone
 * "Triangulos con OOP"
 * 28/5/2015
 */

#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::string;

class Triangulo
{

enum TipoTriangulo{
	EQUILATERO,
	ISOSCELES,
	ESCALENO
};	
	
public:
	Triangulo ();
	string GetTipoTriangulo();
	
private:
	int _longLadoA;
	int _longLadoB;
	int _longLadoC;
	int _TipoTriangulo;
	
	int CalcularTipoTriangulo(int, int, int);
	bool EsTriangulo (int, int, int);
	int SetLado();
};



int main() {

	Triangulo miTriangulo1;
	cout << "El triangulo es " << miTriangulo1.GetTipoTriangulo() << ".\n" << endl;
	
	Triangulo miTriangulo2;
	cout << "El triangulo es " << miTriangulo2.GetTipoTriangulo() << ".\n" << endl;
}



Triangulo::Triangulo(){
	do {
		cout << "Ingrese la longitud del lado A: ";
		_longLadoA = SetLado();
		cout << "Ingrese la longitud del lado B: ";
		_longLadoB = SetLado();
		cout << "Ingrese la longitud del lado C: ";
		_longLadoC = SetLado();
	} while (!EsTriangulo( _longLadoA, _longLadoB, _longLadoC));
	_TipoTriangulo = CalcularTipoTriangulo( _longLadoA, _longLadoB, _longLadoC);
}

int Triangulo::SetLado(){
	int x;

	cin >> x;
	if (!(x > 0)){
		cout << "El valor ingresado es incorrecto. Intente nuevamente: ";
		cin >> x;
	}
}

bool Triangulo::EsTriangulo (int x, int y, int z){
	
	if (!(x <= y + z) && (y <= x + z) && (z <= x + y)){
		cout << "\nLos lados no corresponden a un triangulo en el espacio euclideano."
		<< "\nIntente nuevamente.\n" << endl;
		return false;
	}
	return true;
}

int Triangulo::CalcularTipoTriangulo(int x, int y, int z){

	if ((x == y) && (y == z))
		return EQUILATERO;
	else if ((x == y) || (y == z) || (x == z))
		return ISOSCELES;
	else
		return ESCALENO;
};

string Triangulo::GetTipoTriangulo(){
	switch (_TipoTriangulo){
		case EQUILATERO :
			return "equilatero";
				
		case ISOSCELES :
			return "isosceles";
				
		case ESCALENO:
			return "escaleno";
		
	}
};

#include <iostream>

int GetLadoTriangulo();
bool EsTriangulo (int, int, int);
int DefinirClaseTriangulo (int, int, int);

using namespace std;

enum Triangulo{
EQUILATERO,
ISOSCELES,
ESCALENO
};

int main(){

cout << "Inserte la longitud de los lados del triangulo: " << endl;

int a = GetLadoTriangulo();
int b = GetLadoTriangulo();
int c = GetLadoTriangulo();

if (EsTriangulo(a, b, c)){
	cout << "\nLa figura es un triangulo.\n";
	int claseTriangulo = DefinirClaseTriangulo(a, b, c);
	
	if (claseTriangulo == EQUILATERO)
		cout << "El triangulo es equilatero.";
	else if (claseTriangulo == ISOSCELES)
		cout << "El triangulo es isoceles.";
	else if (claseTriangulo == ESCALENO)
		cout << "El triangulo es escaleno.";}

else
	cout << "\nLa figura no es un triangulo.\n";
}

int GetLadoTriangulo(){
	int x;
	cin >> x;
	return x;
}

bool EsTriangulo(int x, int y, int z){
	return ((x < y + z) && (y < x + z) && (z < x + y));
}

int DefinirClaseTriangulo (int x, int y, int z){
	if ((x==y) && (y==z))
		return EQUILATERO; 
	else if ((x==y) || (x==z) || (y==z))
		return ISOSCELES; 	
	else
		return ESCALENO;	
}

#include <array>
#include <string>
#include <iostream>

using namespace std;

array <unsigned int, 10> Naturales = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void ContarNaturales(char);
void MostrarNaturales();

int main()
{
	cout << "Estimado usuario le vamos a solicitar ingrese una secuencia numerica que finalice con una Z." << endl << "De esta secuencia le informaremos cuantas veces aparecere cada numero natural" << endl << endl;
	char secuencia;
	cin >> secuencia;
	while (secuencia != 'Z') {
		ContarNaturales(secuencia);
		cin >> secuencia;
	}
	MostrarNaturales();
	cout << "Fin del programa...Gracias por participar";
	system("pause");
	return 0;
}

void ContarNaturales(char secuencia) {
	for (int x = 48; x < 58; x++){
		if (x == secuencia)  
		{
			Naturales[x - '0'] += 1;
		}
	}
}

void MostrarNaturales(){
	for (int y = 0; y < 10; y++){
		cout <<endl << endl << "El numero " << y << " aparecio " << Naturales[y] << " veces en la secuencia" << endl << endl ;
	}
}

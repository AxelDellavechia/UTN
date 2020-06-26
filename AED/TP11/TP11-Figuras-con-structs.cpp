//  AED - TP11 -  Figuras_con_structs.cpp
//  TP11 - Figuras con Structs
//  Creado por Grupo 2 el 28/8/15.

#include <iostream>
#include <array>
#include <cmath>
#include "TP11-Funciones.h"

enum nivelMenu {
	MENUPPAL,
	MENUOPERAR,
	MENUOPTRI,
	MENUOPCIR,
	MENUOPREC
};

using namespace std;

array<Circulo, 100> Circulos;
array<Triangulo, 100> Triangulos;
array<Rectangulo, 100> Rectangulos;

Triangulo t;
Circulo c;
Rectangulo r;

char respuesta;
unsigned int numerofig;

istream& mostrarMenu(char& respuesta, int level, unsigned int conttri, unsigned int contcir, unsigned int contrec);
Triangulo& leerTriangulo (Triangulo& t);
Circulo& leerCirculo (Circulo& c);
Rectangulo& leerRectangulo (Rectangulo& r);

void mostrarFiguras(unsigned int contadortri, unsigned int contadorcir, unsigned int contadorrec);
void seleccionarTipoFigura (int menulevel, unsigned int contadortri, unsigned int contadorcir, unsigned int contadorrec);
void seleccionarNumeroFigura (int level, unsigned int contadortri, unsigned int contadorcir, unsigned int contadorrec);

double getPerimetro (Circulo c);
double getArea (Circulo C);

void imprimBanner()
{
	cout << "\t+-------------------------------------+" << endl;
	cout << "\t|Algoritmos y Estructuras de Datos    |" << endl;
	cout << "\t|UTN-FRBA-Curso K1051                 |" << endl;
	cout << "\t|TP No 11 - Figuras con structs       |" << endl;
	cout << "\t+-------------------------------------+\n\n" << endl;
	
}

int main () {
	imprimBanner();	
	nivelMenu level = MENUPPAL;
	unsigned int contadortri = 0;
	unsigned int contadorcir = 0;
	unsigned int contadorrec = 0;

	while (mostrarMenu(respuesta, level, contadortri, contadorcir, contadorrec)){
		switch (respuesta){
			
			case '1':
				Triangulos[contadortri] = leerTriangulo(t);
				contadortri++;
				break;

			case '2':
				Circulos[contadorcir] = leerCirculo(c);
				contadorcir++;
				break;

			case '3':
				Rectangulos[contadorrec] = leerRectangulo(r);
				contadorrec++;
				break;

			case '4':
				mostrarFiguras(contadortri, contadorcir, contadorrec);
				break;	

			case '5':
				seleccionarTipoFigura(level, contadortri, contadorcir, contadorrec);
				break;

			case 'q':
			case 'Q':
				exit(0);
		}
	}
}

istream& mostrarMenu(char& answer, int level, unsigned int conttri, unsigned int contcir, unsigned int contrec)
{
	if (level == MENUPPAL){
		cout << endl << "Indique que accion desea realizar a continuacion:" << endl << endl
		<< "1 - Insertar triangulo" << endl
		<< "2 - Insertar circulo" << endl
		<< "3 - Insertar rectangulo" << endl
		<< "4 - Ver figuras" << endl
		<< "5 - Operar con figuras" << endl
		<< "Q - Salir" << endl << endl;
	}
	
	if (level == MENUOPERAR){
		cout << endl << "Indique sobre que tipo de figura desea operar:" << endl << endl
		<< "1 - Triangulo" << endl
		<< "2 - Circulo" << endl
		<< "3 - Rectangulo" << endl
		<< "4 - Ver figuras" << endl
		<< "Q - Salir" << endl << endl;
	}
	
	if (level == MENUOPTRI){
		seleccionarNumeroFigura (level, conttri, contcir, contrec);
		
		if (numerofig != -1){
			cout << endl << "Indique que accion desea realizar sobre el triangulo seleccionado:" << endl << endl
			<< "1 - Devolver perimetro" << endl
			<< "2 - Devolver area" << endl
			<< "3 - Determinar si es isosceles" << endl
			<< "4 - Determinar si es equilatero" << endl
			<< "5 - Determinar si es escaleno" << endl
			<< "6 - Inscribir el triangulo en un rectangulo" << endl
			<< "7 - Comparar dos triangulos" << endl
			<< "Q - Salir" << endl << endl;
		} else {
			cout << endl << "Inserte cualquier valor para continuar.";
		}
	}
	
	if (level == MENUOPCIR){
		seleccionarNumeroFigura (level, conttri, contcir, contrec);
				
		if (numerofig != -1){
			cout << endl << "Indique que accion desea realizar sobre el circulo seleccionado:" << endl << endl
			<< "1 - Devolver perimetro" << endl
			<< "2 - Devolver area" << endl
			<< "3 - Devolver diametro" << endl
			<< "4 - Incrementar porcentualmente" << endl
			<< "5 - Comparar dos circulos" << endl
			<< "6 - Distancia entre circulos" << endl
			<< "7 - Interseccion entre circulos" << endl
			<< "Q - Salir" << endl << endl;
		} else {
			cout << endl << "Inserte cualquier valor para continuar.";
		}
	}
	
	if (level == MENUOPREC){
		seleccionarNumeroFigura (level, conttri, contcir, contrec);
			
		if (numerofig != -1){
			cout << endl << "Indique que accion desea realizar sobre el rectangulo seleccionado:" << endl << endl
			<< "1 - Devolver perimetro" << endl
			<< "2 - Devolver area" << endl
			<< "3 - Devolver longitud de la diagonal" << endl
			<< "4 - Devolver base del rectangulo" << endl
			<< "5 - Devolver altura del rectangulo" << endl
			<< "6 - Determinar si es cuadrado" << endl
			<< "7 - Comparar dos rectangulos" << endl
			<< "Q - Salir" << endl << endl;
		} else {
			cout << endl << "Inserte cualquier valor para continuar.";
		}
	}
	return (cin >> answer);
}

Triangulo& leerTriangulo (Triangulo& tri){
	cout << "Inserte la coordenadas X del primer punto: ";
	cin >> tri.verticea.posx;
	cout << "Inserte la coordenadas Y del primer punto: ";
	cin >> tri.verticea.posy;
	
	cout << "Inserte la coordenadas X del segundo punto: ";
	cin >> tri.verticeb.posx;
	cout << "Inserte la coordenadas Y del segundo punto: ";
	cin >> tri.verticeb.posy;
	
	cout << "Inserte la coordenadas X del tercer punto: ";
	cin >> tri.verticec.posx;
	cout << "Inserte la coordenadas Y del tercer punto: ";
	cin >> tri.verticec.posy;
	cout << endl;
	
	return tri;
}

Circulo& leerCirculo (Circulo& cir){
	cout << "Inserte la coordenadas X del centro: ";
	cin >> cir.centro.posx;
	cout << "Inserte la coordenadas Y del centro: ";
	cin >> cir.centro.posy;
	
	cout << "Inserte la longitud del radio del circulo: ";
	cin >> cir.radio;
	cout << endl;
	
	return cir;
}

Rectangulo& leerRectangulo (Rectangulo& rec){
	cout << "Inserte la coordenadas X del primer punto: ";
	cin >> rec.verticea.posx;
	cout << "Inserte la coordenadas Y del primer punto: ";
	cin >> rec.verticea.posy;
	
	cout << "Inserte la coordenadas X del segundo punto: ";
	cin >> rec.verticeb.posx;
	cout << "Inserte la coordenadas Y del segundo punto: ";
	cin >> rec.verticeb.posy;
	
	cout << endl;
	
	return rec;
}

void mostrarFiguras(unsigned int contadortri, unsigned int contadorcir, unsigned int contadorrec){
	for (int i = 0; i < contadortri; i++){
		cout << endl;
		cout << "Triangulo numero " << (i + 1) << " :" << endl;
		cout << "Coordenadas del primer vertice: (" << Triangulos[i].verticea.posx << " , " << Triangulos[i].verticea.posy << ")" << endl;
		cout << "Coordenadas del segundo vertice: (" << Triangulos[i].verticeb.posx << " , " << Triangulos[i].verticeb.posy << ")" << endl;
		cout << "Coordenadas del tercer vertice: (" << Triangulos[i].verticec.posx << " , " << Triangulos[i].verticec.posy << ")" << endl << endl;	
	}
	
	for (int i = 0; i < contadorcir; i++){
		cout << endl;
		cout << "Circulo numero " << (i + 1) << " :" << endl;
		cout << "Coordenadas del centro: (" << Circulos[i].centro.posx << " , " << Circulos[i].centro.posy << ")" << endl;
		cout << "Longitud del radio: " << Circulos[i].radio << endl << endl;	
	}
	
	for (int i = 0; i < contadorrec; i++){
		cout << endl;
		cout << "Rectangulo numero " << (i + 1) << " :" << endl;
		cout << "Coordenadas del primer vertice: (" << Rectangulos[i].verticea.posx << " , " << Rectangulos[i].verticea.posy << ")" << endl;
		cout << "Coordenadas del segundo vertice: (" << Rectangulos[i].verticeb.posx << " , " << Rectangulos[i].verticeb.posy << ")" << endl << endl;	
	}
}

void seleccionarTipoFigura (int menulevel, unsigned int conttri, unsigned int contcir, unsigned int contrec){
	menulevel = MENUOPERAR;
	char resp = 0;
	
	do {
		mostrarMenu(resp, menulevel, conttri, contcir, contrec);
		
		switch (resp){
			case '1':
				menulevel = MENUOPTRI;
				mostrarMenu(respuesta, menulevel, conttri, contcir, contrec);
				break;
				
			case '2':
				menulevel = MENUOPCIR;
				mostrarMenu(respuesta, menulevel, conttri, contcir, contrec);
				break;
				
			case '3':
				menulevel = MENUOPREC;
				mostrarMenu(respuesta, menulevel, conttri, contcir, contrec);
				break;
				
			case '4':
				mostrarFiguras(conttri, contcir, contrec);
				break;
				
			case 'q':
			case 'Q':
				break;
		}
	}
	while (resp == '4');
}

void seleccionarNumeroFigura(int tipoFigura, unsigned int contadortri, unsigned int contadorcir, unsigned int contadorrec){
	int limiteFigura;
	switch (tipoFigura){
		case MENUOPTRI:
			cout << endl << "Indique el numero de triangulo sobre el que desea operar.";
			limiteFigura = contadortri;
			break;
		case MENUOPCIR:
			cout << endl << "Indique el numero de circulo sobre el que desea operar.";
			limiteFigura = contadorcir;
			break;
		case MENUOPREC:
			cout << endl << "Indique el numero de rectangulo sobre el que desea operar.";
			limiteFigura = contadorrec;
			break;
	}
	cout << endl << "Actualmente existen " << limiteFigura << " figuras de este tipo:" << endl;
	
	if (limiteFigura == 0){
		numerofig = -1;
		return;
	}
		
	cin >> numerofig;
	cout << endl << "Figura numero " << numerofig << " seleccionada." << endl;
	numerofig--;
		
	while (numerofig > limiteFigura){
		cout << endl << "La figura numero " << numerofig << " no existe." << endl
		<< "Por favor, indique nuevamente el numero de la figura sobre la cual desea operar: " << endl;
		cin >> numerofig;	
	}
}

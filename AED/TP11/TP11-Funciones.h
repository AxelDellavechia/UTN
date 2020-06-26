# ifndef FUNCIONES_H
# define FUNCIONES_H

struct Punto2D {
	double posx;	// proyeccion del punto sobre el eje x
	double posy;	// proyeccion del punto sobre el eje y
};

struct Circulo {
	double radio;
	Punto2D centro;
};

struct Triangulo {
	Punto2D verticea;
	Punto2D verticeb;
	Punto2D verticec;
};

struct Rectangulo {
	Punto2D verticea;
	Punto2D verticeb;
};

// Círculo
double perimetroUnCirculo (Circulo &C);
double areaUnCirculo (Circulo &C);
double diametroUnCirculo (Circulo &C);
Circulo incrementoPorcentualUnCirculo (Circulo &C);
void compararDosCirculos (Circulo &C1, Circulo &C2);
double distanciaEntreCirculos (Circulo &C1, Circulo &C2);
bool interseccionEntreCirculos (Circulo &C1, Circulo &C2);

//Rectángulo
double perimetroUnRectangulo (Rectangulo &r);
double areaUnRectangulo (Rectangulo &r);
double LongitudDiagonalUnRectangulo (Rectangulo &r);
double baseRectangulo (Rectangulo &r);
double alturaRectangulo (Rectangulo &r);
bool esCuadrado (Rectangulo &r);
void compararDosRectangulos (Rectangulo &r1, Rectangulo &r2);

//Triangulo
double perimetroTriangulo (Triangulo &t);
double areaTriangulo (Triangulo &t);
bool EsEquilatero (Triangulo &t);
bool EsIsosceles (Triangulo &t);
bool EsEscaleno (Triangulo &t);
Rectangulo rectanguloDeUnTriangulo (Triangulo &t);
void compararTriangulos (Triangulo &t1, Triangulo &t2);

# endif

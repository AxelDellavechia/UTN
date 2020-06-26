
# ENUM 

#### C++ permite que los programadores creen sus propios *tipos de datos* de varias formas, de las cuales una es mediante la utilización de *tipos enumerados*.

#### Los tipos enumerados (también llamados *enumeraciones*) son tipos de datos en los cuales cada valor posible se define como una constante simbólica (llamada *enumerador*). Los tipos enumerados se declaran mediante la palabra clave *enum*.

#### Los enumeradores se separan entre sí mediante comas. La definición de cada enumeración se encierra entre llaves y se cierra con un punto y coma.

Por ejemplo:

```cpp
enum Color { ROJO, AMARILLO, VERDE };
```

#### Cada enumerador es asignado un valor de acuerdo a su lugar en la lista. Siguiendo el ejemplo anterior: ####

```cpp
enum Color

{

ROJO,           // Se le asigna el valor 0

AMARILLO,      // Se le asigna el valor 1

> VERDE         // Se le asigna el valor 2

};
```

#### Al asignar un enumerador a otra variable, o al insertar un enumerador en un flujo de salida, su valor es evaluado como un entero. Así, por ejemplo, el siguiente código: ####

```cpp
> int miColor = VERDE;

> std::cout << miColor << “ ” << VERDE;
```

#### Tendría como salida en pantalla: ####

> 2 2

#### Debemos observar que si trabajamos con varias enumeraciones al mismo tiempo, existe la posibilidad de cometer graves errores semánticos. Por ejemplo, este fragmento de código: ####

```cpp
int main(){

enum Peras { PERA_ROJA, PERA_AMARILLA };

enum Olmos { OLMO_COMUN, OLMO_CHINO, OLMO_DE_MONTANA};

Olmo miOlmo = OLMO_CHINO;

Pera miPera = PERA_AMARILLA;

if (miOlmo == miPera) // Ambos valores son comparados como enteros.

>>> std::cout << “Hemos comparado exitosamente peras con olmos”;

else

>>> std::cout << “Imposible comparar peras con olmos”;

}
```

#### Tendría una salida, digamos, que no es la esperada.

#### Estos problemas se solucionan fácilmente mediante el uso de *enum class*, una característica incorporada en C++ 11. Cada enum class se ubica en un *namespace* diferente y las distintas enum class no pueden ser comparadas entre sí como si fueran enteros: 

```cpp
enum class Peras { PERA_ROJA, PERA_AMARILLA };

enum class Olmos { OLMO_COMUN, OLMO_CHINO, OLMO_DE_MONTANA };

Olmo miOlmo = Olmos::OLMO_CHINO;

Pera miPera = Peras::PERA_AMARILLA;

if (miOlmo == miPera) // Error de compilador, ambos tipos no pueden ser comparados.

>>> std::cout << “Hemos comparado exitosamente peras con olmos”;

else

>>> std::cout << “Imposible comparar peras con olmos”;
```

#### Las enumeraciones son particularmente útiles para documentar qué es lo que hace nuestro código, cada vez que debamos representar una serie de estados alternativos, recordando siempre que la representación implícita es de tipo entero. Por ejemplo:

```cpp
enum Triangulo{EQUILATERO, ISOSCELES, ESCALENO };

int main(){

using namespace std;

int a = GetLadoTriangulo();

int b = GetLadoTriangulo();

int c = GetLadoTriangulo();

int claseTriangulo = DefinirClaseTriangulo(a, b, c);

if (claseTriangulo == EQUILATERO)

>>> cout << “El triangulo es equilatero”;

else if (claseTriangulo == ISOSCELES)

>>> cout << “El triangulo es isoceles”;

else (claseTriangulo == ESCALENO)

>>> cout << “El triangulo es escaleno”;
}

int DefinirClaseTriangulo (int x, int y, int z){

if ((x==y) && (y==z))

>>> return EQUILATERO;

else if ((x==y) || (x==z) || (y==z))

>>> return ISOSCELES;

else

>>> return ESCALENO;

}
```

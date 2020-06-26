## Consigna del trabajo práctico

Realizar un intérprete del lenguaje MICRO.
Para ello se debe utilizar LEX y YACC (cualquier producto que lo implemente) y lenguaje C o
C++ o hacerlo puramente en lenguaje C.
La entrada de las sentencias debe ser por entrada estándar o desde un archivo que el usuario
pueda seleccionar.
El sistema debe, ante la lectura de cada sentencia, indicar que tipo de sentencia fue
reconocida y evaluarla, caso contrario, si hubo error.

## Descripción del lenguaje MICRO


El único tipo de dato es entero.

Todos los identificadores son declarados implícitamente y con una longitud máxima de 32
caracteres.

Los identificadores deben comenzar con una letra y están compuestos de letras y dígitos.


Las constantes son secuencias de dígitos (números enteros).

Las sentencias pueden ser:
```javascript

Asignación: Tiene la forma ID := Expresión; 
Expresión es infija y se construye con identificadores, constantes y los operadores + y – 
; los paréntesis están permitidos.

Entrada/Salida, cuyo formato es: 
                                  leer (lista de IDs); 
                                  escribir (lista de Expresiones);
Cada sentencia termina con un "punto y coma" (;).
El cuerpo de un programa está delimitado por las palabras reservadas inicio y fin.
inicio, fin, leer y escribir son palabras reservadas y deben escribirse en minúscula.
```

#### Gramática Léxica 


```javascript
<objetivo> -> <programa> FDT
<programa> -> INICIO <listaSentencias> FIN
<listaSentencias> -> <sentencia> {<sentencia>}
<sentencia> -> ID ASIGNACIÓN <expresión> PUNTOYCOMA |
LEER PARENIZQUIERDO <listaIdentificadores> PARENDERECHO PUNTOYCOMA |
ESCRIBIR PARENIZQUIERDO <listaExpresiones> PARENDERECHO PUNTOYCOMA
<listaIdentificadores> -> ID {COMA ID}
<listaExpresiones> -> <expresión> {COMA <expresión>}UTN – FRBA SSL Ing. Pablo Méndez
<expresión> -> <primaria> {<operadorAditivo> <primaria>}
<primaria> -> ID | CONSTANTE | PARENIZQUIERDO <expresión> PARENDERECHO
<operadorAditivo> -> SUMA | RESTA
```

## Para datos extras,ejemplos consultar el archivo [Datos_Extra.txt](./Datos_Extra.txt)

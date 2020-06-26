%{
	
/********************
*Declaraciones en C*
********************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct datoMixto {
                    char  simbolo[32] ;
                    int  valor ;
} datoMixto;

typedef struct string {
                    char  palabra[32] ;
} string;

datoMixto * tablaDeSimbolos;

int * operacionesEscribir ;

string * identificadores;

extern int yylineno;
extern int yylex();
extern int yyparse(void);
extern FILE *yyin;

void yyerror (char *s);

void errorIdentificadorNoDeclarado(char * palabra);
void errorPalabraReservada(char * palabra);

void cargarPalabrasReservasEnTabla( datoMixto * tabla);
void cargarIdentificadores(string * conjuntoIden , datoMixto * tabla);

void mostrarOperaciones (int * tablaOperaciones) ;

void identificarSentencia(int);

int escribir=0;
int leer=0;
int contadorEscritura = 0;
int contadorIdentificadores = 0;
int contadorPalabras = 0;

%}

/********************
*Declaraciones en Bison*
********************/

%error-verbose

%union	{
			int constante;
			char identificador[32];
		}

%token  <identificador> IDENTIFICADOR 
%token  <constante> CONSTANTE SIMB_MAS SIMB_MENOS SIMB_ASIG SIMB_COMA SIMB_PYC PARENT_IZQUIERDO PARENT_DERECHO LEER ESCRIBIR INICIO FIN

%type <identificador> sentencia listaSentencias;
%type <constante> expresion primaria primaria2 listaExpresiones;

%left   SIMB_MAS SIMB_MENOS

%start programa

%%

/********************
*Reglas Gramaticales*
********************/

programa:	INICIO 	{
					tablaDeSimbolos = (datoMixto*) malloc(4*sizeof( datoMixto*));
					contadorIdentificadores=4;
					cargarPalabrasReservasEnTabla(tablaDeSimbolos) ;
					} listaSentencias FIN 	{
												printf("\n Entrada correcta! Finalizando programa, gracias y vuelva pronto");
												exit(0);
											};

listaSentencias:  listaSentencias sentencia | sentencia

sentencia:	IDENTIFICADOR SIMB_ASIG expresion SIMB_PYC 
				{
					tablaDeSimbolos = (datoMixto*) malloc(sizeof( datoMixto*));		   
					strcpy( tablaDeSimbolos[contadorIdentificadores].simbolo, $1);
					tablaDeSimbolos[contadorIdentificadores].valor = $3; 
					identificarSentencia(0); 
					//printf("\n El identificador es : %s y su posicion en la tabla de simbolos es %d y tiene un valor de : %d",tablaDeSimbolos[contadorIdentificadores].simbolo,contadorIdentificadores,tablaDeSimbolos[contadorIdentificadores].valor);
					contadorIdentificadores++;
				} |
			LEER {leer=1;} PARENT_IZQUIERDO listaIdentificadores PARENT_DERECHO SIMB_PYC 
				{
					identificarSentencia(1); 
					cargarIdentificadores(identificadores, tablaDeSimbolos);
					leer=0;
				} |
			ESCRIBIR {escribir=1;} PARENT_IZQUIERDO listaExpresiones PARENT_DERECHO SIMB_PYC 
				{ 
					
					identificarSentencia(2); 
					mostrarOperaciones(operacionesEscribir); 
					escribir=0;
				}
				
expresion: expresion SIMB_MAS primaria {$$ = $1 + $3;} | expresion SIMB_MENOS primaria {$$ = $1 - $3; }  | primaria {$$ = $1;}

listaIdentificadores: 	IDENTIFICADOR 
									{	
										if (leer == 1) {
											identificadores = (string*) malloc( sizeof(string*) );
											strcpy(identificadores[contadorPalabras].palabra, $1);
											printf("\n El identificador en memoria es %s \n",identificadores[contadorPalabras].palabra);
											contadorPalabras++;
											
										}	
									} SIMB_COMA listaIdentificadores { } | 
						IDENTIFICADOR 
									{ 
										if(leer == 1) {
											identificadores = (string*) malloc( sizeof(string*) );
											strcpy(identificadores[contadorPalabras].palabra, $1);
											printf("\n El identificador en memoria es %s \n",identificadores[contadorPalabras].palabra);
											contadorPalabras++;
										}	
									}
								
listaExpresiones: 	expresion 	{ 
									if(escribir == 1) 	{
															operacionesEscribir = ( int *) malloc( sizeof( int *));
															operacionesEscribir[contadorEscritura] = $1;
															contadorEscritura++;
														}
								} SIMB_COMA listaExpresiones { } | 
					expresion 
						{ 
							if(escribir == 1) {
													operacionesEscribir = ( int *) malloc( sizeof( int *));
													operacionesEscribir[contadorEscritura] = $1;
													contadorEscritura++;
										}
						}
primaria:	primaria2 {$$ = $1;}
						
primaria2: 	IDENTIFICADOR 	{ 
								//printf("La cantidad de identificadores es: %d", contadorIdentificadores);
								int aux ; int pos ; int identificador ;
								  
								for ( aux = 0; aux < contadorIdentificadores ; aux ++){
									if (strcmp(tablaDeSimbolos[aux].simbolo,$1) == 0){
										pos = aux ;
										identificador = 1;											  
									}								 
								} if (identificador == 0) {
										errorIdentificadorNoDeclarado($1);
									} if (tablaDeSimbolos[pos].valor == -1) {
												errorPalabraReservada($1);
										} 	else 	{
													$$ = tablaDeSimbolos[pos].valor;
											}
							}	| 	CONSTANTE {$$ = $1;} | PARENT_IZQUIERDO expresion PARENT_DERECHO {$$ = $2;}

%%

/********************
*Codigo C Adicional*
********************/

void yyerror(char *s) {
   printf("Se ha detectado un error Sintactico.\n");
}

int main(int num_args, char** args) {
	if(num_args > 3) {
		printf("Cantidad de Parametros incorrectos"); 
	} else {
				if (num_args == 2) {
					FILE* archivo = fopen(args[1],"r");
					if(archivo == NULL) {
						printf("No se puedo abrir el archivo: ", args[1]);
					} else	{
								yyin = archivo;
								yyparse();
								fclose(archivo);
								free(operacionesEscribir);
								free(identificadores);
								free(tablaDeSimbolos);				
							}
				} else	{
							printf("Ingreso de las sentencias de forma manual\n");
							printf("        Un programa en Micro debe tener la siguiente estructura\n");
							printf("  *******************************************************************\n");
							printf("	inicio\n");
							printf("	sentencias del tipo leer | escribir | asignacion , \n	todas deben finalizar con ;\n");
							printf("	fin\n");
							printf("  *******************************************************************\n");
							printf(" Ya puede comenzar a tipear su programa\n\n");
							yyparse();
							free(operacionesEscribir);
							free(identificadores);
							free(tablaDeSimbolos);			
						}
			}
}


void cargarIdentificadores(string * conjuntoIden , datoMixto * tabla) {
	int aux = 0;
   //printf("\n La cantidad de identificadores es %d\n",contadorPalabras);
	for ( aux ; aux < contadorPalabras ; aux++) {
     printf("\n Cargando el valor para el identificador nro %d : ",aux);
        scanf ("%d",&tabla[contadorIdentificadores].valor);
		strcpy( tabla[contadorIdentificadores].simbolo, conjuntoIden[aux].palabra);
		//printf("\n La tabla de simbolos ahora contiene en la fila %d,%d el valor %s,%d : ", contadorIdentificadores , contadorIdentificadores , tabla[contadorIdentificadores].simbolo, tabla[contadorIdentificadores].valor) ;
		contadorIdentificadores++;
	}
}

void cargarPalabrasReservasEnTabla(datoMixto * tabla) {
    strcpy( tabla[0].simbolo, "inicio");
    tabla[0].valor = -1 ;

    strcpy( tabla[1].simbolo, "fin");
    tabla[1].valor = -1 ;

    strcpy( tabla[2].simbolo, "leer");
    tabla[2].valor = -1 ;

    strcpy( tabla[3].simbolo, "escribir");
    tabla[3].valor = -1 ;
	
	//printf("\n Tabla iniciada correctamente con los siguientes valores:\n Fila 0,0 %s,%d \n Fila 1,1 %s,%d \n Fila 2,2 %s,%d \n Fila 3,3 %s,%d \n",tabla[0].simbolo,tabla[0].valor,tabla[1].simbolo,tabla[1].valor,tabla[2].simbolo,tabla[2].valor,tabla[3].simbolo,tabla[3].valor);
}

void identificarSentencia(int tipo){
	switch(tipo){
		case 0: printf("\n Se interpreto una sentencia de ASIGNACION (:=), en linea %d \n", yylineno);
			break;
		case 1: printf("\n Se interpreto una sentencia de LECTURA (leer), en linea %d \n", yylineno);
			break;
		case 2: printf("\n Se interpreto una sentencia de ESCRITURA (escribir), en linea %d \n", yylineno);
			break;
	}
}

void errorIdentificadorNoDeclarado(char * palabra){
	printf("\n Error semantico: %s no es un identificador declarado, en linea %d",palabra,yylineno);
	exit(1);
}

void errorPalabraReservada(char * palabra){
	printf("\n Error semantico: %s  es palabra reservada, en linea %d", palabra , yylineno);
	exit(2);
}

void mostrarOperaciones (int * tablaOperaciones) {
	int aux;
	for( aux = 0 ; aux < contadorEscritura ; aux++){
			printf("\n La escritura Nro %d es: %d \n", aux, tablaOperaciones[aux]);
    }
}

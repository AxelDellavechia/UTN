//  AED - TP8 -  Fechas.cpp
//  TP8 - Fecha
//  Creado por Grupo 2 el 6/6/15.

#include <iostream>

using namespace std;

bool esFecha (unsigned int, unsigned int, int);
bool esBisiesto (int);

enum Meses {
	ENERO = 1, FEBRERO, MARZO,
	ABRIL, MAYO, JUNIO,
	JULIO, AGOSTO, SEPTIEMBRE,
	OCTUBRE, NOVIEMBRE, DICIEMBRE
};

int main() {   
    unsigned int diaIngresado;
    cout << "Ingrese d\241a:" << endl;
    cin >> diaIngresado;   
    unsigned int mesIngresado;
    cout << "Ingrese mes:" << endl;
    cin >> mesIngresado;   
    int anoIngresado;
    cout << "Ingrese a\xA4o:" << endl;
    cin >> anoIngresado;  
    
     if ((esFecha(diaIngresado, mesIngresado, anoIngresado)) && esBisiesto(anoIngresado))
        	cout << endl << "La fecha ingresada es valida y el a\xA4o es bisiesto" << endl;
        else if ((esFecha(diaIngresado, mesIngresado, anoIngresado)) && !(esBisiesto(anoIngresado)))
				cout << endl << "La fecha ingresada es valida y el a\xA4o no es bisiesto" << endl;
	else       
        cout << "La fecha ingresada no es valida" << endl;   
}
    
}

bool esBisiesto(int ano){
    if (ano % 4 == 0 && (ano % 100 != 0 || ano % 400 == 0))
        return true;
    else
        return false; 
}

bool esFecha (unsigned int dia, unsigned int mes, int ano){  
    
    if ((ano == 0) || (mes == 0) || (dia == 0))
    	return false;
	if ((mes > 12) || (dia > 31))
        return false;
    
	if (mes != FEBRERO){  
        if (dia <= 30) //Cantidad de dias que cualquier mes menos febrero puede llevar        
        	return true;
        else if (dia == 31) {              
        	switch (mes) {
	            case ENERO:
	            case MARZO:
	            case MAYO:
	            case JULIO:
	            case AGOSTO:
	            case OCTUBRE:
	            case DICIEMBRE: // Recoge los meses que tienen 31 dias
	                return true;
	                break;
	            default: //Recoge los meses que no tienen 31 dias
	            	return false;
        	}
    	}
	}
	else { //Se fija si es febrero, mes conflictivo en ano bisiesto 
			if ((dia <= 28) || ((dia == 29) && (esBisiesto(ano)))) //Si es bisiesto         
            	return true;
        	else
				return false;
	}
}						

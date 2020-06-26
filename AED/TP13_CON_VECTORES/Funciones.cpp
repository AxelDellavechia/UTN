#include <iostream>
#include <iomanip>
#include <array>
#include "Funciones.h"

using namespace std;

void llenarTabla (vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas){
    
    double ventas;
    
  
    cout << "Llenando tabla..." << endl << endl;
    
     
    for(int i = 0; i<filas; i++){
        for(int j = 0; j<columnas; j++){
            cout<< "Ingrese cantidad de ventas del \n Vendedor n: " << (i + 1)
            << " || En la region n: " << (j + 1) << "\n";
            cin >> ventas;
            unamatriz [i] [j] = ventas;
        }
    }
    
    
}
void mostrarTabla (const vector<vector<double>> &matriz, unsigned int filaInicio, unsigned int columnaInicio,
					unsigned int filaFin, unsigned int columnaFin){
	
	double ventasTotalesPorRegion = 0.0;
	
	for(int i = filaInicio; i < filaFin; i++){
	// Escribe fila inicial:
		if (i == filaInicio){
	    	cout << left << setw(18) << "Region No. ";
	    	for(int j = columnaInicio; j < columnaFin; j++)
	    		cout  << right << setw(8) << (j + 1);
	// Separador:
			cout << endl << setfill('-') << setw(18 + 8*(columnaFin - columnaInicio)) << "-" << setfill(' ') << endl;
			}
	// Escribe fila con la informacion sobre las ventas:
		cout << left << setw(13) << "Vendedor No. " << (i + 1) << ":   ";
	    for(int k = columnaInicio; k < columnaFin; k++){
	       	cout << right << setw(8) << fixed << setprecision(2) << matriz.at(i).at(k);
	        }	    
	    cout << endl;
	}
	// Separador:
	cout << setfill('-') << setw(18 + 8*(columnaFin - columnaInicio)) << "-" << setfill(' ') << endl;
	// Escribe la fila final:
	cout << left << setw(18) << "Total por region:";
	    for(int l = columnaInicio; l < columnaFin; l++){
	    	for (int m = filaInicio; m < filaFin; m++)
				ventasTotalesPorRegion += matriz.at(m).at(l);
		cout << right << setw(8) << fixed << setprecision(2) << ventasTotalesPorRegion;
	    ventasTotalesPorRegion = 0;
	    }
}

void mostrarTodaLaTabla(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas){
    
    cout << endl << "Mostrando tabla..." << endl << endl;
    mostrarTabla (unamatriz, 0, 0, filas, columnas);
        
}


void ventasTotales(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas){
    double ventasTotales = 0.0;
    	
	cout << endl << "El total de las ventas es: ";
	    for(int i = 0; i < filas; i++)
	    	for (int j = 0; j < columnas; j++)
				ventasTotales += unamatriz.at(i).at(j);
	cout << fixed << setprecision(2) << ventasTotales;
}

void ventasPorVendedor(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas){
    double ventasTotales = 0.0;
	
	cout << endl;
	for(int i = 0; i < filas; i++){
		for (int j = 0; j < columnas; j++)
			ventasTotales += unamatriz.at(i).at(j);
	cout << left << setw(13) << "Vendedor No. " << (i + 1) << ":   "
	<< right << setw(8) << fixed << setprecision(2) << ventasTotales << endl;
	ventasTotales = 0.0;
	}
}

void ventasPorRegion(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas){
    double ventasTotales = 0.0;
    
    cout << endl;
	for (int i = 0; i == 0; i++){
	    cout << left << setw(18) << "Region No. ";
	    for(int j = 0; j < columnas; j++)
	    	cout  << right << setw(8) << (j + 1);
	// Separador:
			cout << endl << setfill('-') << setw(18 + 8*columnas) << "-" << setfill(' ') << endl;
	}
	cout << left << setw(18) << "Total por region:";
    for(int k = 0; k < columnas; k++){
    	for (int l = 0; l < filas; l++)
			ventasTotales += unamatriz.at(l).at(k);
	cout << right << setw(8) << fixed << setprecision(2) << ventasTotales;
    ventasTotales = 0;
    }
}
void regionConMasVentas(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas){

	int regionMayorVenta;
	double ventaMaxima = 0.0;
	double ventasTotales = 0.0;
    
    cout << endl;
    for(int i = 0; i < columnas; i++){
    	for (int j = 0; j < filas; j++){
			ventasTotales += unamatriz.at(j).at(i);
			if (ventasTotales >= ventaMaxima){
				ventaMaxima = ventasTotales;
				regionMayorVenta = i;
			}
		}
    ventasTotales = 0;
    }
    cout << "La region con mas ventas es la region " << regionMayorVenta + 1 << " ( "<< fixed << setprecision(2) << ventaMaxima << " )";
}

void vendedorConMasVentas(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas){
 
	int vendedorMayorVenta;
	double ventaMaxima = 0.0;
	double ventasTotales = 0.0;
    
    cout << endl;
    for(int i = 0; i < filas; i++){
    	for (int j = 0; j < columnas; j++){
			ventasTotales += unamatriz.at(i).at(j);
			if (ventasTotales >= ventaMaxima){
				ventaMaxima = ventasTotales;
				vendedorMayorVenta = i;
			}
		}
    ventasTotales = 0;
    }
    cout << "El vendedor con mas ventas es el vendedor No. " << vendedorMayorVenta + 1 << " ( "<< fixed << setprecision(2) << ventaMaxima << " )";
}

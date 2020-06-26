#include <iostream>
#include <vector>
#include "Funciones.h"

using namespace std;

int main() {
   
    unsigned int filas;
	unsigned int columnas;
	
	cout << "Ingrese el numero de vendedores: ";
    cin >> filas;
    cout << "Ingrese el numero de regiones: ";
    cin >> columnas;
	cout << endl;
	
    vector<vector<double>> matriz (filas, vector<double> (columnas));
    
    llenarTabla(matriz, filas, columnas);
    
    int opcionMenu;
    
    do {
   cout << endl << endl;
        cout << "Seleccione opcion:\n"
        
             << "1:Mostrar tabla\n"
             << "2:Informar total de ventas\n"
             << "3:Informar ventas por vendedor\n"
             << "4:Informar ventas por region\n"
             << "5:Informar region con mas ventas\n"
             << "6:Informar vendedor con mas ventas\n"
             << "0:Salir\n";
        
        cin >> opcionMenu;
        
        switch(opcionMenu){
            
            case 1:
                mostrarTodaLaTabla(matriz, filas, columnas);
                break;
            case 2:
                ventasTotales(matriz, filas, columnas);
                break;
            case 3:
                ventasPorVendedor(matriz, filas, columnas);
                break;
            case 4:
                ventasPorRegion(matriz, filas, columnas);
                break;
            case 5:
                regionConMasVentas(matriz, filas, columnas);
                break;
            case 6:
                vendedorConMasVentas(matriz, filas, columnas);
                break;
            default:
            	break;
        }

        
        
        
    }while (opcionMenu != 0);
    
    
}

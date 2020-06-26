#include <iostream>
#include <array>
using namespace std;

void llenarTabla (array<array<int,2>,2> &unaTabla);
void mostrarTabla(const array<array<int,2>,2> &unaTabla);
void ventasTotales(const array<array<int,2>,2> &unaTabla);
void ventasPorVendedor(const array<array<int,2>,2> &unaTabla);
void ventasPorRegion(const array<array<int,2>,2> &unaTabla);
void regionConMasVentas(const array<array<int,2>,2> &unaTabla);
void vendedorConMasVentas(const array<array<int,2>,2> &unaTabla);


int main() {
    
    array<array<int,2>,2> tabla;
    
    llenarTabla(tabla);
    
    int opcionMenu;
    
    do {
        
        cout << "Seleccione opcion:\n"
        
             << "1:Mostrar tabla\n"
             << "2:Informar total de ventas\n"
             << "3:Informar ventas por vendedor\n"
             << "4:Informar ventas por region\n"
             << "5:Informar region con mas ventas\n"
             << "6:Informar vendedor con mas ventas\n"
             << "7:Sobreescribir tabla\n"
             << "0:Salir\n";
        
        cin >> opcionMenu;
        
        switch(opcionMenu){
            
            case 1:
                mostrarTabla(tabla);
                break;
            case 2:
                ventasTotales(tabla);
                break;
            case 3:
                ventasPorVendedor(tabla);
                break;
            case 4:
                ventasPorRegion(tabla);
                break;
            case 5:
                regionConMasVentas(tabla);
                break;
            case 6:
                vendedorConMasVentas(tabla);
                break;
            case 7:
                llenarTabla(tabla);
                break;
            case 0:
                break;
            default:
                cout << "Opcion erronea. Terminando programa.";
                return 0;
                
        }
        
    }while (opcionMenu != 0);
    
    return 0;
    
    
}



void mostrarTabla(const array<array<int,2>,2> &unaTabla){
    
    cout << "Mostrando tabla...\n";
    
    for(int i = 0; i<2; i++){
        for(int j = 0; j<2; j++){
            cout << unaTabla.at(i).at(j) << "  ";
        }
        cout << "\n" <<endl;
    }
    
}

void llenarTabla (array<array<int,2>,2> &unaTabla){
    
    cout << "Llenando tabla...\n";
    
    double ventas;
    
    for(int i = 0; i<unaTabla.size(); i++){
        for(int j = 0; j<unaTabla.at(i).size(); j++){
            cout<< "Ingrese cantidad de ventas del \n Vendedor n: " << i
            << " || En la region n: " << j << "\n";
            cin >> ventas;
            unaTabla.at(i).at(j) = ventas;
        }
    }
    
    
}

void ventasTotales(const array<array<int,2>,2> &unaTabla){
    
    int ventasTotales = 0;
    for(int i = 0; i<unaTabla.size(); i++){
        for(int j = 0; j<unaTabla.at(i).size(); j++){
            ventasTotales += unaTabla.at(i).at(j);
        }
    }
    
    cout << "Se vendio un total de: $" << ventasTotales << "\n";
    
    
}

void ventasPorVendedor(const array<array<int,2>,2> &unaTabla){
    
    int totalVendedor = 0;
    for(int i = 0; i<unaTabla.size(); i++){
        for(int j = 0; j<unaTabla.at(i).size(); j++){
            totalVendedor += unaTabla.at(i).at(j);
        }
        cout << "El vendedor: " << i << " Vendio: $"<< totalVendedor << "\n" << endl;
        totalVendedor = 0;
    }
    
    
}

void ventasPorRegion(const array<array<int,2>,2> &unaTabla){
    int totalRegion = 0;

    int i = 0;
    for(int j = 0; j<unaTabla.at(i).size(); j++){
        for(int i = 0; i<unaTabla.size(); i++){
            totalRegion += unaTabla.at(i).at(j);
        }
        cout << "En la region: " << j << " se vendio: $"<< totalRegion << "\n" << endl;
        totalRegion = 0;
    }
    

}

void regionConMasVentas(const array<array<int,2>,2> &unaTabla){
    
    array<int,2>totalesRegiones;
    int totalRegion = 0;
    int i = 0;
    for(int j = 0; j<unaTabla.at(i).size(); j++){
        for(int i = 0; i<unaTabla.size(); i++){
            totalRegion += unaTabla.at(i).at(j);
        }
        totalesRegiones.at(j) = totalRegion;
        totalRegion = 0;
    }
    
    int maximaRegion = 0;
    int regionConMasVentas = 0;
    
    for(i=0; i<totalesRegiones.size();++i){
        if(totalesRegiones.at(i)>maximaRegion){
            maximaRegion = totalesRegiones.at(i);
            regionConMasVentas = i;
        }
    }
    
    cout << "La region con mas ventas es la region: " << regionConMasVentas << " habiendo vendido: $" << maximaRegion << "\n";
    
}

void vendedorConMasVentas(const array<array<int,2>,2> &unaTabla){
    array<int,2>totalesVendedores;
    int totalVendedor = 0;
    for(int i = 0; i<unaTabla.size(); i++){
        for(int j = 0; j<unaTabla.at(i).size(); j++){
            totalVendedor += unaTabla.at(i).at(j);
        }
        totalesVendedores.at(i) = totalVendedor;
        totalVendedor = 0;
    }
    
    int maximaVenta = 0;
    int mejorVendedor = 0;
    for(int i=0; i<totalesVendedores.size();++i){
        if(totalesVendedores.at(i)>maximaVenta){
            maximaVenta = totalesVendedores.at(i);
            mejorVendedor = i;
        }
    }
    
    cout << "El vendedor con mas ventas es el n: " << mejorVendedor << " habiendo vendido: $" << maximaVenta<<"\n";

    
}

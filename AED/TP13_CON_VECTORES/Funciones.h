#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <vector>
using std::vector;

void llenarTabla (vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas);
void mostrarTabla (const vector<vector<double>> &unamatriz, unsigned int filaInicio, unsigned int columnaInicio,
					unsigned int filaFin, unsigned int columnaFin);
void mostrarTodaLaTabla(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas);
void ventasTotales(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas);
void ventasPorVendedor(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas);
void ventasPorRegion(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas);
void regionConMasVentas(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas);
void vendedorConMasVentas(const vector<vector<double>> &unamatriz, unsigned int filas, unsigned int columnas);

#endif

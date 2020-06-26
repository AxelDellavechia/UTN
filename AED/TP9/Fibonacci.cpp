#include <iostream>
#include <array>
using namespace std;

unsigned fib1 (unsigned, std::array<unsigned, 21>);
unsigned long long fib2 (unsigned);

int main(){
	
	static const std::array <unsigned, 21> f = {0,1,1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597,2584,4181,6765};
	
	cout << "Bienvenido al graficador de series de Fibonacci.\n"
		<< "Inserte la cantidad de numeros de la serie de Fibonacci que desea ver: ";
	unsigned int limite = 0; //cantidad de elementos de la serie de Fibonacci a mostrar 
	cin >> limite;
	cout << endl << "Estos son los primeros " << limite << " numeros de la serie de Fibonacci:" << endl;
	
    for (unsigned i = 0; i < limite; i++) {
    	try{
		cout << "F(" << i << ")" << '=' << fib1(i, f) << endl;	
		}
		catch (exception e){
			cout << "F(" << i << ")" << '=' << fib2(i) << endl;	
		}
	}
    
}

unsigned fib1(unsigned n, std::array<unsigned, 21> arrayFib){
		return arrayFib.at(n);
}

unsigned long long fib2(unsigned n){
	
	if (n == 0)
		return 0;
	if (n == 1)
		return 1;
	return (fib2(n-1) + fib2(n-2));
}

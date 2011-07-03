// tstcomplex.cpp -- testar klassen complex
#include <complex>
#include <iostream>

using std::cout;
using std::endl;

typedef std::complex<double> complex;

int main(void)
{
   complex test1, test2;
   test1 = complex(5,6);
   test2 = 2;
   complex test3(4,8);
	
   cout << "Addition av " << test1 << " och " << test2 << " ger:\n";
   cout << test1 + test2 << endl;
   cout << "Subtrahering av " << test1 << " och " << test3 << " ger:\n";
   cout << test1 - test3 << endl;
   cout << "Multiplicering av " << test1 << " och " << test2 << " ger:\n";
   cout << test1*test2 << endl;
   cout << "Multiplicering av " << test1 << " och " << test3 << " ger:\n";
   cout << test1*test3 << endl;
   cout << "Kvadrering av " << test1 << " ger:\n";
   cout << test1*test1 << endl;
   cout << "Division av " << test1 << " med " << test2 << " ger:\n";
   cout << test1/test2 << endl;
   cout << "Division av " << test1 << " med " << test3 << " ger:\n";
   cout << test1/test3 << endl;
   return 0;
}

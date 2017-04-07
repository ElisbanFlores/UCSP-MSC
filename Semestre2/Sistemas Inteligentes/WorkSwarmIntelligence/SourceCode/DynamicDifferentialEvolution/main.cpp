#include <iostream>
#include <math.h>
#include <time.h>

using namespace std;

#include "CPattern.h"
#include "CDynamicalDifferentialEvolution.h"



#define PI           3.14159265358979323846  
#define MINIMUM 	 0
#define MAXIMUM		 1

template <typename T>
class Fitness {
public:
	double operator()(T _pattern){
		double max = 0.0;
		for (int i = 0; i < _pattern._features.size(); i++){
			max += _pattern._features[i];
		}
		return max/_pattern._features.size();
	}
};

template <typename T>
class FitnessAckley {
	double a;
	double b;
	double c;
public:
	double operator()(T _pattern){
		a = 20;
		b = .2;
		c = 2*PI;
		double tmp1 = 0.0;
		double tmp2 = 0.0;
		int size = _pattern._features.size();
		for (int i = 0; i < _pattern._features.size(); i++){
			tmp1 +=  _pattern._features[i] *  _pattern._features[i];
			tmp2 +=  cos(c *  _pattern._features[i]);
		}
		return -a * exp(-b * sqrt(tmp1/size)) - exp(tmp2/size) + a + exp(1);
	}
};

template <typename T>
class FitnessSchwefel {
public:
	double operator()(T _pattern){
		double tmp1 = 0.0;
		int size = _pattern._features.size();
		for (int i = 0; i < _pattern._features.size(); i++){
			tmp1 +=  _pattern._features[i] *  sin(sqrt(fabs(_pattern._features[i])));
		}
		return (418.9829 * size) - tmp1;
	}
};

template <typename T>
class funcion3 {
public:
	double operator()(T _pattern){
		double tmp1 = 0.0;
		int size = _pattern._features.size();
		for (int i = 0; i < _pattern._features.size(); i++){
			tmp1 +=  _pattern._features[i] *  _pattern._features[i];
		}
		return .5 - ((pow(sin(tmp1),2)-.5) / pow((1.0 + 0.001 * tmp1),2));
	}
};

// Doc
// max limite de aleatorios
// min limite de aleatorios
// Cpattern < tipo, dimension, numero decimales> 
// fitness < tipo de patron >
// CDynamicalDifferentialEvolution < tipo de patron, numero de poblacion, funcion Fitness>
// Al llamar
// evolution_t _p(min_value,max_value,.4,.3,50); // min, max, factor de mutacion, factor de combinacion, iteraciones


// for Ackley d=10 
/*
double  max_value= 32.7680;
double  min_value= -32.7680;
typedef CPattern<double,10,1000> pattern_t;
typedef FitnessAckley<pattern_t> fitness_t;
typedef CDynamicalDifferentialEvolution<pattern_t,5000,fitness_t,MINIMUM> evolution_t;
*/

//for Ackley d=2
/*double  max_value = 32.7680;
double  min_value = -32.7680;
typedef CPattern<double, 2, 100> pattern_t;
typedef FitnessAckley<pattern_t> fitness_t;
typedef CDynamicalDifferentialEvolution<pattern_t, 100, fitness_t, MINIMUM> evolution_t;
*/


/*
// for Schwefel d=2
double  max_value= 500;
double  min_value= -500;
typedef CPattern<double,2,50> pattern_t;
typedef FitnessSchwefel<pattern_t> fitness_t;
typedef CDynamicalDifferentialEvolution<pattern_t,200,fitness_t,MINIMUM> evolution_t;
*/

// for Schwefel d=10
/*
double  max_value = 500;
double  min_value = -500;
typedef CPattern<double, 10,0> pattern_t;
typedef FitnessSchwefel<pattern_t> fitness_t;
typedef CDynamicalDifferentialEvolution<pattern_t,17500, fitness_t, MINIMUM> evolution_t;
*/


// for funcion3  d=2 d =10
double  max_value= 100;
double  min_value= -100;
typedef CPattern<double,10,100> pattern_t;
typedef funcion3<pattern_t> fitness_t;
typedef CDynamicalDifferentialEvolution<pattern_t,2000,fitness_t,MAXIMUM> evolution_t;


int main (int, char*[]){
	srand(time(NULL));
	int nRep = 100;
	while (nRep-- > 0)
	{
		//evolution_t _p(min_value,max_value,.2,.7,50); // for Ackley d2 y d10		
		//evolution_t _p(min_value,max_value,.2,.6,50); // for Schwefel d2
		//evolution_t _p(min_value, max_value, .01, .4, 50); // for Schwefel d10		
		evolution_t _p(min_value, max_value, .2, .8, 50); // for funcion3 d2 y d10
	}
	cout << "Finished";
	
	getchar();
	return true;
}
#ifndef C_DYNAMICAL_DIFFERENTIAL_EVOLUTION_H
#define C_DYNAMICAL_DIFFERENTIAL_EVOLUTION_H

#include <vector>
#include <iostream>
#include "RandomUniform.h"

using namespace std;

template <typename P, int NP, typename FO, int TYPE> 
class CDynamicalDifferentialEvolution {
	protected:
	typedef P					pattern_t;
	typedef FO					function_t;
	typedef vector<pattern_t> 	container_t;

	public:
	container_t _population; 
	container_t _noise_population; 
	container_t _trial_population; 
	function_t 	_fitness;
	pattern_t 	_optimal;

	public:
	CDynamicalDifferentialEvolution(double _pmin, double _pmax, double _f, double _gr, int iterations){
		for (int i = 0; i < NP; i++){
			_population.push_back(pattern_t(_pmin, _pmax));
		}		
		//cout << *this << endl;// 
		while(iterations>=0){
			get_optimal();
			mutation(_f);
			recombination(_gr);
			selection();
			reset();
			
			if ((50-iterations) % 5 == 0)
				cout << get_optimal_value() << ";";				
			iterations--;
		}
		cout << endl;
		//cout << *this << endl;
		
	}

	void get_optimal (void){
		double value = 0.0;
		if (TYPE == 0) { // minimun
			value = numeric_limits<double>::max();
			for (int i = 0; i <  NP; i++){
				if (_fitness(_population[i]) < value){
					value = _fitness(_population[i]);
					_optimal = _population[i];
				}
			}
		}
		if (TYPE == 1) { // maximum
			value = numeric_limits<double>::min();
			for (int i = 0; i <  NP; i++){
				if (_fitness(_population[i]) > value){
					value = _fitness(_population[i]);
					_optimal = _population[i];
				}
			}
		}
	}

	void mutation (double _factor){
		for (int i = 0; i <  NP; i++){
			int a = (int)RandomUniform(0.0, (double)NP-1.0);//rand() % NP;
			int b = (int)RandomUniform(0.0, (double)NP-1.0);//rand() % NP;
			_noise_population.push_back(
				_optimal + ((_population[a] - _population[b]) * _factor)
			);
		}
	}

	void recombination (double _factor){
		for (int i = 0; i < NP; i++){
			pattern_t _tmp;
			for (int k = 0; k < _population[0]._features.size(); k++){
				double rand_value = RandomUniform(0.0, 1.0);//(double)(rand() % 10)/10.0;
				if (rand_value < _factor)
					_tmp._features[k] = _noise_population[i]._features[k];
				else	
					_tmp._features[k] = _population[i]._features[k];
			}
			_trial_population.push_back(_tmp);
		}
	}

	void selection (void){
		for (int i = 0; i < NP; i++){
			if (TYPE == 0) { // minimun
				if (_fitness(_trial_population[i]) < _fitness(_population[i]))
					_population[i] = _trial_population[i];
				if (_fitness(_population[i]) < _fitness(_optimal))
					_optimal = _population[i];					
			}
			if (TYPE == 1) { // maximum 
				if (_fitness(_trial_population[i]) > _fitness(_population[i]))
					_population[i] = _trial_population[i];
				if (_fitness(_population[i]) > _fitness(_optimal))
					_optimal = _population[i];								
			}
		}
	}

	void reset(void){
		_noise_population.clear();
		_trial_population.clear();
	}

	double get_optimal_value(void){
		return _fitness(_optimal);
	}
 	

	template <typename _P, int _NP, typename _FO, int _TYPE>
	friend ostream& operator<< (ostream& out, CDynamicalDifferentialEvolution<_P,_NP,_FO,_TYPE>& diff_ev){
		out << endl << "Population: " << endl;
		for (int i = 0; i <  diff_ev._population.size(); i++)
			cout << diff_ev._population[i] << endl;
		
		out << endl << "Noise Population: " << endl;			
		for (int i = 0; i <  diff_ev._noise_population.size(); i++)
			cout << diff_ev._noise_population[i] << endl;
		
		out << endl << "Trial Population: " << endl;
		for (int i = 0; i <  diff_ev._trial_population.size(); i++)
			cout << diff_ev._trial_population[i] << endl;
		out << "------ " << endl;
		return out;
	}

};


#endif
#ifndef CDIFFERENTIAL_EVOLUTION_H
#define CDIFFERENTIAL_EVOLUTION_H

#include <vector>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "boost/random.hpp"
#include "RandomUniform.h"

using namespace std;

typedef boost::uniform_int<> NumberDistribution; 
typedef boost::mt19937 RandomNumberGenerator; 
typedef boost::variate_generator<RandomNumberGenerator&,NumberDistribution> Generator; 

template <typename P, int NP, typename FO, int TYPE> 
class CDynamicalDifferentialEvolutionBoost {
	protected:
	typedef P					pattern_t;
	typedef FO					function_t;
	typedef vector<pattern_t> 	container_t;

	public:
	container_t _population; 
	container_t _noise_population; 
	container_t _trial_population; 
	function_t 	_fitness;
	int 		_nthrds;
	pattern_t 	_optimal;

	public:

	CDynamicalDifferentialEvolutionBoost(double _pmin, double _pmax, double _f, double _gr, int iterations){
		for (int i = 0; i < NP; i++){
			_population.push_back(pattern_t(_pmin, _pmax));
		}
		_nthrds = boost::thread::hardware_concurrency();

		//cout << *this << endl;
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

	void mutate (double _factor, int limit_x, int limit_y){
			for (int i = limit_x; i <  limit_y; i++){
				int a = (int)RandomUniform(0.0, (double)NP-1.0); //rand() % NP;
				int b = (int)RandomUniform(0.0, (double)NP-1.0);//rand() % NP;
				int c = (int)RandomUniform(0.0, (double)NP-1.0);//rand() % NP;
				_noise_population[i] = _optimal + ((_population[a] - _population[b]) * _factor);
			}
	}

	void mutation (double _factor){
		_noise_population = _population;
		vector<boost::thread*> thrds;
		int scope = NP/_nthrds;
		int pos = 0;
		for (int i = 0; i < _nthrds; i++){
			boost::thread * th = new boost::thread(&CDynamicalDifferentialEvolutionBoost<P,NP,FO,TYPE>::mutate,this,_factor,pos,pos+scope);
			thrds.push_back(th);
			pos += scope;
		}
		for (int i = 0; i < _nthrds; i++){
			thrds[i]->join();
		}
	}

	void combine (double _factor, int limit_x, int limit_y){
		for (int i = limit_x; i <  limit_y; i++){
			pattern_t _tmp;
			for (int k = 0; k < _population[0]._features.size(); k++){
				double rand_value = RandomUniform(0.0, 1.0);//(double)(rand() % 10)/10.0;
				if (rand_value < _factor)
					_tmp._features[k] = _noise_population[i]._features[k];
				else	
					_tmp._features[k] = _population[i]._features[k];
			}
			_trial_population[i] = _tmp;
		}
	}

	void recombination (double _factor){
		random_shuffle(_population.begin(), _population.end());
		random_shuffle(_noise_population.begin(), _noise_population.end());
		_trial_population = _population;
		vector<boost::thread*> thrds;
		int scope = NP/_nthrds;
		int pos = 0;
		for (int i = 0; i < _nthrds; i++){
			boost::thread * th = new boost::thread(&CDynamicalDifferentialEvolutionBoost<P,NP,FO,TYPE>::combine,this,_factor,pos,pos+scope);
			thrds.push_back(th);
			pos += scope;
		}
		for (int i = 0; i < _nthrds; i++){
			thrds[i]->join();
		}
	}


	void select (int limit_x, int limit_y){
		for (int i = limit_x; i <  limit_y; i++){
			if (TYPE == 0) { // minimun
				if (_fitness(_trial_population[i]) < _fitness(_population[i]))
					_population[i] = _trial_population[i];
				
				if (_fitness(_population[i]) < _fitness(_optimal)){
					boost::mutex::scoped_lock scoped_lock(boost::mutex);
					_optimal = _population[i];
				}				
			}
			if (TYPE == 1) { // maximum 
				if (_fitness(_trial_population[i]) > _fitness(_population[i]))
					_population[i] = _trial_population[i];
					
				if (_fitness(_population[i]) > _fitness(_optimal)){
					boost::mutex::scoped_lock scoped_lock(boost::mutex);
					_optimal = _population[i];					
				}
			}
		}
	}

	void selection (void){
		vector<boost::thread*> thrds;
		int scope = NP/_nthrds;
		int pos = 0;
		for (int i = 0; i < _nthrds; i++){
			boost::thread * th = new boost::thread(&CDynamicalDifferentialEvolutionBoost<P,NP,FO,TYPE>::select,this,pos,pos+scope);
			thrds.push_back(th);
			pos += scope;
		}
		for (int i = 0; i < _nthrds; i++){
			thrds[i]->join();
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
	friend ostream& operator<< (ostream& out, CDynamicalDifferentialEvolutionBoost<_P,_NP,_FO,_TYPE>& diff_ev){
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
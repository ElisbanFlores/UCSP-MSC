#ifndef CPATTERN_H
#define CPATTERN_H

#include <iostream>
#include <vector>
#include "RandomUniform.h"

using namespace std;

template <typename T, int SIZE, int SCOPE>
class CPattern {

	protected:
	typedef T value_t;
	typedef vector<value_t> 	container_t;

	public:
	container_t _features;

	public:

		CPattern(const CPattern& _pattern){
				_features = _pattern._features;
		}

		CPattern(double MIN  = 0.0, double MAX = 0.0){
			_features = container_t(SIZE,0.0);
			for (int i = 0; i < _features.size(); i++){
				_features[i] = RandomUniform(MIN, MAX);//(double)MIN + (double)(rand()%(int)SCOPE)/(double)SCOPE * (double)(MAX - MIN); 
			}
		}

		CPattern operator-(CPattern __p){
			CPattern tmp; 
			for (int i = 0; i < _features.size(); i++)
				tmp._features[i] = _features[i] - __p._features[i];
			return tmp;
		}

		CPattern operator+(CPattern __p){
			CPattern tmp; 
			for (int i = 0; i < _features.size(); i++)
				tmp._features[i] = _features[i] + __p._features[i];
			return tmp;
		}

		template <typename _T, int _SIZE, int _SCOPE>
		friend bool operator<(const CPattern<_T,_SIZE,_SCOPE>& _p, const CPattern<_T,_SIZE,_SCOPE>& __p){
			double _tmp = 0.0, __tmp = 0.0;
			for (int i = 0; i < _SIZE; i++){
				_tmp += _p._features[i] * _p._features[i];
				__tmp += __p._features[i] * __p._features[i];
			}
			return sqrt(_tmp) < sqrt(__tmp);
		}		

		CPattern operator*(double _factor){
			CPattern tmp; 
			for (int i = 0; i < _features.size(); i++)
				tmp._features[i] = _features[i] * _factor;
			return tmp;
		}

		template <typename _T, int _SIZE, int _SCOPE>
		friend ostream& operator<< (ostream& out, CPattern<_T,_SIZE,_SCOPE> _pattern){
			for (int i = 0; i < _pattern._features.size(); i++){
				out << _pattern._features[i] << " ";
			}
			return out;
		}

};

#endif
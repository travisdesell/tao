#ifndef TAO_UTIL_H
#define TAO_UTIL_H

#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

template <typename T>  
void string_to_vector(string s, vector<T> &v);

template <typename T>
string vector_to_string(const vector<T> *v);

template <typename T>
string vector_to_string(const vector<T> &v);


template <typename T>
void string_to_vector_2d(string s, T (*convert)(const char*), vector< vector<T> > &v);

template <typename T>
string vector_2d_to_string(const vector< vector<T> > &v);

#endif

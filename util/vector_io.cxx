#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <stdexcept>

#include "stdint.h"
#include "vector_io.hxx"

using namespace std;

template <typename T>  
void string_to_vector_2d(string s, T (*convert)(const char*), vector< vector<T> > &v) { 
    v.clear();

    int32_t begin = 1;
    int32_t v_start = 0, v_end = 0;

    while (v_start != std::string::npos) {
        v_start = s.find('[', begin);
        v_end = s.find(']', v_start);

        cout << "v_start: " << v_start << ", v_end: " << v_end << endl;

        vector<T> current;
        string_to_vector(s.substr(v_start, v_end - v_start), convert, current);

        v.push_back(current);
        cout << "pushed vector: " << vector_to_string( *(v.end()) );
    }
}

template <typename T>
string vector_2d_to_string(const vector< vector<T> > &v) {
    ostringstream oss;
    
    oss << "[ ";
    for (uint32_t i = 0; i < v.size(); i++) {
        if (i > 0) oss << ", ";
        oss << vector_to_string(v[i]);
    }
    oss << "]";

    return oss.str();
}

void split_string(string s, string delimiters, vector<string> &splits) {
    splits.clear();

    stringstream stringStream(s);
    string line;

    while (getline(stringStream, line)) {
        std::size_t prev = 0, pos;

        while ((pos = line.find_first_of(delimiters, prev)) != std::string::npos) {
            if (pos > prev) {
                splits.push_back(line.substr(prev, pos - prev));
            }

            prev = pos + 1;
        }

        if (prev < line.length()) {
            splits.push_back(line.substr(prev, std::string::npos));
        }
    }
}

template <typename T>  
void string_to_vector(string s, vector<T> &v) { 
    v.clear();

    vector<string> splits;
    split_string(s, "[], ", splits);

    for (uint32_t i = 0; i < splits.size(); i++) {
        if (splits[i].size() > 0) {
            T value;
            stringstream(splits[i]) >> value;
            v.push_back(value);
        }
    }
}

template <>
void string_to_vector(string s, vector<string> &v) {
    v.clear();

    vector<string> splits;
    split_string(s, "[], ", splits);

    for (uint32_t i = 0; i < splits.size(); i++) {
        if (splits[i].size() > 0) {
            v.push_back(splits[i]);
        }
    }
}

template <typename T>
string vector_to_string(const vector<T> &v) {
    ostringstream oss;

    oss.precision(15);
    oss << "[";
    for (unsigned int i = 0; i < v.size(); i++) {
        if (i > 0) oss << ", ";
        oss << v[i];
    }   
    oss << "]";

    return oss.str();
}

template <typename T>
string vector_to_string(const vector<T> *v) {
    return vector_to_string(*v);
}

template string vector_to_string<bool>(const vector<bool> *v);
template string vector_to_string<bool>(const vector<bool> &v);
template string vector_to_string<int>(const vector<int> *v);
template string vector_to_string<int>(const vector<int> &v);
template string vector_to_string<uint32_t>(const vector<uint32_t> *v);
template string vector_to_string<uint32_t>(const vector<uint32_t> &v);
template string vector_to_string<uint64_t>(const vector<uint64_t> *v);
template string vector_to_string<uint64_t>(const vector<uint64_t> &v);
template string vector_to_string(const vector<double> *v);
template string vector_to_string(const vector<double> &v);
template string vector_to_string(const vector<string> *v);
template string vector_to_string(const vector<string> &v);

template string vector_2d_to_string(const vector< vector<double> > &v);

template void string_to_vector<double>(string s, vector<double> &v);
template void string_to_vector<uint64_t>(string s, vector<uint64_t> &v);


#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <stdexcept>

#include "stdint.h"
#include "vector_io.hxx"

#include <boost/algorithm/string.hpp>

#ifdef BOOST_NO_EXCEPTIONS
namespace boost
{
	extern inline void throw_exception(std::exception const& e) {
		std::cerr << "uncaught exception: " << e.what() << std::endl;
		std::exit(1);
	}
}
#endif

using namespace std;

template <typename T>  
void string_to_vector_2d(string s, T (*convert)(const char*), vector< vector<T> > &v) { 
    v.clear();

    uint32_t begin = 1;
    uint32_t v_start = 0, v_end = 0;

    while (v_start != string::npos) {
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

template <typename T>  
void string_to_vector(string s, vector<T> &v) { 
    v.clear();

    vector<std::string> split_string;
    boost::split(split_string, s, boost::is_any_of("[], "));

    for (uint32_t i = 0; i < split_string.size(); i++) {
        if (split_string[i].size() > 0) {
            T value;
            stringstream(split_string[i]) >> value;
            v.push_back(value);
        }
    }
}

template <>
void string_to_vector(string s, vector<string> &v) {
    v.clear();

    vector<std::string> split_string;
    boost::split(split_string, s, boost::is_any_of("[], "));

    for (uint32_t i = 0; i < split_string.size(); i++) {
        if (split_string[i].size() > 0) {
            v.push_back(split_string[i]);
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


#include <cfloat>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "particle_swarm.hxx"
#include "differential_evolution.hxx"

#include "parameter_sweep.hxx"

//from undvc_common
#include "arguments.hxx"


using namespace std;


/**
 *  The following are used to read the data elements from the given file
 */
int observations;
vector<double> cleanprices;
vector<double> dirtyprices;
vector<double> netwages;
vector<double> time_endowments;
vector<double> virtual_incs;
vector<double> leisuregoods;
vector<double> netwage_insts;
vector<double> cleangoods;
vector<double> dirtygoods;

vector<double> t1s;
vector<double> t2s;
vector<double> t3s;
vector<double> t4s;
vector<double> t5s;
vector<double> t6s;

void initialize_data(char *filename) {
    ifstream data_file(filename, ifstream::in );
    if (data_file.is_open()) {
        string str;
        getline(data_file, str);

        int observation_number;
        double cleanprice;
        double dirtyprice;
        double netwage;
        double time_endowment;
        double virtual_inc;
        double leisuregood;
        double netwage_inst;
        double cleangood;
        double dirtygood;

//        while (data_file.good()) {
        while ( data_file >> cleanprice >> dirtyprice >> netwage >> time_endowment >> virtual_inc >> leisuregood >> netwage_inst >> cleangood >> dirtygood >> observation_number ) {
            observations++;

            cleanprices.push_back(cleanprice);
            dirtyprices.push_back(dirtyprice);
            netwages.push_back(netwage);
            time_endowments.push_back(time_endowment);
            virtual_incs.push_back(virtual_inc);
            leisuregoods.push_back(leisuregood);
            netwage_insts.push_back(netwage_inst);
            cleangoods.push_back(cleangood);
            dirtygoods.push_back(dirtygood);

//            cout << "read: " << cleanprice << " " << dirtyprice << " " << netwage << " " << " " << time_endowment << " " << virtual_inc << " " << leisuregood << " " << netwage_inst << " " << cleangood << " " << dirtygood << " " << observation_number << endl;

            t1s.push_back( cleanprice * (dirtyprice / cleanprice) );
            t2s.push_back( dirtyprice * (dirtyprice / cleanprice) );
            t3s.push_back( netwage * (dirtyprice / cleanprice) );
            t4s.push_back( cleanprice * (netwage / cleanprice) );
            t5s.push_back( dirtyprice * (netwage / cleanprice) );
            t6s.push_back( netwage * (netwage / cleanprice) );
        }
        data_file.close();
    }
}

/*
%Order of Arguments
%prices = [cleanprice, dirtyprice, netwage]
%qty = [cleangood, dirtygood, leisuregood]
%param = [A_1, A_2, A_3, a, b, c]

%Parameter Estimation - Marginal Rate of Substitution (Y For X)

function f = mrsyx(prices, qty, param)
%Numerator
%num = ( A_1 * ( cleangood - a ) ) + ( A_3 * ( leisuregood - c ) )
%Denominator
%den = ( A_1 * ( dirtygood - b ) ) + ( A_2 * ( leisuregood - c ) )
f = num / den;
*/

double mrsyx(double cleangood, double dirtygood, double leisuregood, double A_1, double A_2, double A_3, double a, double b, double c) {
    double num = (A_1 * (cleangood - a)) + (A_3 * (leisuregood - c));
    double den = (A_1 * (dirtygood - b)) + (A_2 * (leisuregood - c));
    return num / den;
}

/*
%File Purpose: NonSep Util #3 - mrslx.m
%prices = [cleanprice, dirtyprice, netwage]
%qty = [cleangood, dirtygood, leisuregood]
%param = [A_1, A_2, A_3, a, b, c]

%Parameter Estimation - Marginal Rate of Substitution (l For X)

function f = mrslx(prices, qty, param)
%Numerator
%num = ( A_2 * ( cleangood - a ) ) + ( A_3 * ( dirtygood - b ) )
%Denominator
%den = ( A_1 * ( dirtygood - b ) ) + ( A_2 * ( leisuregood - c ) ) 
f = num / den;
*/

double mrslx(double cleangood, double dirtygood, double leisuregood, double A_1, double A_2, double A_3, double a, double b, double c) {
    double num = (A_2 * (cleangood - a)) + (A_3 * (dirtygood - b));
    double den = (A_1 * (dirtygood - b)) + (A_2 * (leisuregood - c));
    return num / den;
}


/*
%Order of Arguments
%prices = [cleanprice, dirtyprice, netwage]
%qty = [cleangood, dirtygood, leisuregood]
%param = [A_1, A_2, A_3, a, b, c]

%Parameter Estimation - Fitness Function 1: FOCs

%From Data Set
prices = [cleanprice, dirtyprice, netwage];
qty = [cleangood, dirtygood, leisuregood];

%Parameters To Solve For
param = [A_1, A_2, A_3, a, b, c];

%term1 = ( cleanprice * ( ( dirtyprice / cleanprice ) - mrsyx(prices, qty, param) ) ) ^ 2
term1 = ( prices(1) * ( ( prices(2) / prices(1) ) - mrsyx(prices, qty, param) ) ) ^ 2;

%term2 = ( dirtyprice * ( ( dirtyprice / cleanprice ) - mrsyx(prices, qty, param) ) ) ^ 2
term2 = ( prices(2) * ( ( prices(2) / prices(1) ) - mrsyx(prices, qty, param) ) ) ^ 2;

%term3 = ( netwage * ( ( dirtyprice / cleanprice ) - mrsyx(prices, qty, param) ) ) ^ 2
term3 = ( prices(3) * ( ( prices(2) / prices(1) ) - mrsyx(prices, qty, param) ) ) ^ 2;

%term4 = ( cleanprice * ( ( netwage / cleanprice ) - mrslx(prices, qty, param) ) ) ^ 2
term4 = ( prices(1) * ( ( prices(3) / prices(1) ) - mrslx(prices, qty, param) ) ) ^ 2;

%term5 = ( dirtyprice * ( ( netwage / cleanprice ) - mrslx(prices, qty, param) ) ) ^ 2
term5 = ( prices(2) * ( ( prices(3) / prices(1) ) - mrslx(prices, qty, param) ) ) ^ 2;

%term6 = ( netwage * ( ( netwage / cleanprice ) - mrslx(prices, qty, param) ) ) ^ 2
term6 = ( prices(3) * ( ( prices(3) / prices(1) ) - mrslx(prices, qty, param) ) ) ^ 2;

f = term1 + term2 + term3 + term4 + term5 + term6;
%f should be calculated for each observation,
%which we then want to find the parameter values which minimize it,
%minimize \sum_{i=1}^n f_i (where n is the total number of
%observations)
*/

double fitness1(const vector<double> &parameters) {
    double A_1 = parameters[0];         /* measure relationship between goods  */
    double A_2 = parameters[1];
    double A_3 = parameters[2];
    double a = parameters[3];           /* minimum consumption parameters */
    double b = parameters[4];
    double c = parameters[5];

//    cout << "evaluating fitness with: " << A_1 << " " << A_2 << " " << A_3 << " " << a << " " << b << " " << c << endl;

    double term1, term2, term3, term4, term5, term6;

    double sum = 0;
    double m1, m2;
    for (int i = 0; i < observations; i++) {
        m1 = mrsyx(cleangoods[i], dirtygoods[i], leisuregoods[i], A_1, A_2, A_3, a, b, c);
        m2 = mrslx(cleangoods[i], dirtygoods[i], leisuregoods[i], A_1, A_2, A_3, a, b, c);

//        term1 = (cleanprices[i] * (dirtyprices[i] / cleanprices[i]) - m1;
        term1 = t1s[i] - m1;
        term1 *= term1;
//        cout << "term1: " << term1 << endl;

//        term2 = (dirtyprices[i] * (dirtyprices[i] / cleanprices[i] ) - m1;
        term2 = t2s[i] - m1;
        term2 *= term2;
//        cout << "term2: " << term2 << endl;

//        term3 = (netwages[i] * (dirtyprices[i] / cleanprices[i] ) - m1;
        term3 = t3s[i] - m1;
        term3 *= term3;
//        cout << "term3: " << term3 << endl;

//        term4 = (cleanprices[i] * (netwages[i] / cleanprices[i]) - m2;
        term4 = t4s[i] - m2;
        term4 *= term4;
//        cout << "term4: " << term4 << endl;

//        term5 = (dirtyprices[i] * (netwages[i] / cleanprices[i] ) - m2;
        term5 = t5s[i] - m2;
        term5 *= term5;
//        cout << "term5: " << term5 << endl;

//        term6 = (netwages[i] * (netwages[i] / cleanprices[i] ) - m2;
        term6 = t6s[i] - m2;
        term6 *= term6;
//        cout << "term6: " << term6 << endl;

        sum += term1 + term2 + term3 + term4 + term5 + term6;

//        cout << "sum: " << sum << endl;
    }

    return -sum;        //returning negative sum as the optimization method maximizes the value
}


/**
 *  New demand functions
 */

double demand(char which, double time_endowment, double virtual_inc, double cleanprice, double dirtyprice, double netwage, double A_1, double A_2, double A_3, double a, double b, double c) {
    switch(which) {
        case 'x':
            {
                double num1  =  (A_3 * cleanprice) - (A_2 * dirtyprice) - (A_1 * netwage);
                double num2a =  (netwage * (time_endowment - c)) + virtual_inc;
                double num2b =  (cleanprice * a) + (dirtyprice * b);
                double num2  =  num2a - num2b;
                double num   =  A_3 * num1 * num2;
                
                double den1 =  ((A_2 * dirtyprice) * (A_2 * dirtyprice)) + ((A_1 * netwage) * (A_1 * netwage)) + ((A_3 * cleanprice) * (A_3 * cleanprice));
                double den2 =  2 * A_2 * A_3 * cleanprice * dirtyprice;
                double den3 =  2 * A_1 * A_3 * cleanprice * netwage;
                double den4 =  2 * A_1 * A_2 * dirtyprice * netwage;
                double den  =  den1 - den2 - den3 - den4;

                return (num / den) + a;
            }
        case 'y':
            {
                double num1  =  (A_2 * dirtyprice) - (A_3 * cleanprice) - (A_1 * netwage);
                double num2a =  (netwage * (time_endowment - c)) + virtual_inc;
                double num2b =  (cleanprice * a) + (dirtyprice * b);
                double num2  =  num2a - num2b;
                double num   =  A_2 * num1 * num2;
                
                double den1 =  ((A_2 * dirtyprice) * (A_2 * dirtyprice)) + ((A_1 * netwage) * (A_1 * netwage)) + ((A_3 * cleanprice) * (A_3 * cleanprice));
                double den2 =  2 * A_2 * A_3 * cleanprice * dirtyprice;
                double den3 =  2 * A_1 * A_3 * cleanprice * netwage;
                double den4 =  2 * A_1 * A_2 * dirtyprice * netwage;
                double den  =  den1 - den2 - den3 - den4;

                return (num / den) + b;
            }
        case 'l':
            {
                double num1  =  (A_1 * netwage) - (A_3 * cleanprice) - (A_2 * dirtyprice);
                double num2a =  (netwage * (time_endowment - c)) + virtual_inc;
                double num2b =  (cleanprice * a) + (dirtyprice * b);
                double num2  =  num2a - num2b;
                double num   =  A_1 * num1 * num2;
                
                double den1 =  ((A_2 * dirtyprice) * (A_2 * dirtyprice)) + ((A_1 * netwage) * (A_1 * netwage)) + ((A_3 * cleanprice) * (A_3 * cleanprice));
                double den2 =  2 * A_2 * A_3 * cleanprice * dirtyprice;
                double den3 =  2 * A_1 * A_3 * cleanprice * netwage;
                double den4 =  2 * A_1 * A_2 * dirtyprice * netwage;
                double den  =  den1 - den2 - den3 - den4;

                return (num / den) + c;
            }
    }
    return '0';
}

double fitness2(const vector<double> &parameters) {
    double A_1 = parameters[0];         /* measure relationship between goods  */
    double A_2 = parameters[1];
    double A_3 = parameters[2];
    double a = parameters[3];           /* minimum consumption parameters */
    double b = parameters[4];
    double c = parameters[5];

    double sum, demandx, demandy, demandl;

    double t1, t2, t3, t4, t5, t6, t7, t8, t9;

    sum = 0;
    for (int i = 0; i < observations; i++) {
        demandx = demand('x', time_endowments[i], virtual_incs[i], cleanprices[i], dirtyprices[i], netwages[i], A_1, A_2, A_3, a, b, c);
        demandy = demand('y', time_endowments[i], virtual_incs[i], cleanprices[i], dirtyprices[i], netwages[i], A_1, A_2, A_3, a, b, c);
        demandl = demand('l', time_endowments[i], virtual_incs[i], cleanprices[i], dirtyprices[i], netwages[i], A_1, A_2, A_3, a, b, c);

        t1 = cleanprices[i] * (demandx - cleangoods[i]);
        t1 *= t1;
        t2 = dirtyprices[i] * (demandx - cleangoods[i]);
        t2 *= t2;
        t3 = netwages[i]    * (demandx - cleangoods[i]);
        t3 *= t3;

        t4 = cleanprices[i] * (demandy - dirtygoods[i]);
        t4 *= t4;
        t5 = dirtyprices[i] * (demandy - dirtygoods[i]);
        t5 *= t5;
        t6 = netwages[i]    * (demandy - dirtygoods[i]);
        t6 *= t6;

        t7 = cleanprices[i] * (demandl - leisuregoods[i]);
        t7 *= t7;
        t8 = dirtyprices[i] * (demandl - leisuregoods[i]);
        t8 *= t8;
        t9 = netwages[i]    * (demandl - leisuregoods[i]);
        t9 *= t9;

        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
    }

    return -sum;
}


int main(int number_arguments, char **argv) {
    vector<string> arguments(argv, argv + number_arguments);

    if (number_arguments < 2 || argument_exists(arguments, "--help") || argument_exists(arguments, "-h") || argument_exists(arguments, "-?")) {
        cerr << "Run the program as follows:" << endl;
        cerr << "    " << arguments[0] << " <data file> [--ps | --de] [search arguments]" << endl;   //arguments    [0] is the name of the program that was run

        exit(0);
    }

    initialize_data(argv[1]);
    /*
     * The following run different optimization methods.
     */

    /* best from parameter sweep:
     *  best parameters:    -1 -4 -5 0 0 0
     *  best fitness:       -1.78073e+08
     */

    int number_parameters = 6;
    vector<double> min_bound(number_parameters, 0);
    vector<double> max_bound(number_parameters, 0);

    min_bound[0] = -5;
    max_bound[0] = 5;
    min_bound[1] = -5;
    max_bound[1] = 5;
    min_bound[2] = -5;
    max_bound[2] = 5;
    min_bound[3] = 0;
//    max_bound[3] = 200;
    max_bound[3] = 2000;
    min_bound[4] = 0;
//    max_bound[4] = 25;
    max_bound[4] = 250;
    min_bound[5] = 0;
    max_bound[5] = 20;

    string search_type;
    get_argument(arguments, "--search_type", true, search_type);

    if (search_type.compare("ps") == 0) {
        ParticleSwarm ps(min_bound, max_bound, arguments);
        ps.iterate(fitness2);

    } else if (search_type.compare("de") == 0) {
        DifferentialEvolution de(min_bound, max_bound, arguments);
        de.iterate(fitness2);

    } else if (search_type.compare("sweep") == 0) {
        vector<double> step_size(6, 0);
        step_size[0] = 1;
        step_size[1] = 1;
        step_size[2] = 1;
        step_size[3] = 20;
        step_size[4] = 5;
        step_size[5] = 5;

        parameter_sweep(min_bound, max_bound, step_size, fitness1);

    } else {
        fprintf(stderr, "Improperly specified search type: '%s'\n", search_type.c_str());
        fprintf(stderr, "Possibilities are:\n");
        fprintf(stderr, "    de     -       differential evolution\n");
        fprintf(stderr, "    ps     -       particle swarm optimization\n");
        exit(0);
    }

    return 0;
}

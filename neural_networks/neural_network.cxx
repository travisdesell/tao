#include <algorithm>
using std::random_shuffle;

#include <cmath>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <iomanip>
using std::setw;
using std::setprecision;

#include <fstream>
using std::ios;
using std::ofstream;
using std::ifstream;
using std::istreambuf_iterator;

#include <limits>
using std::numeric_limits;

#include <sstream>
using std::ostringstream;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "./neural_networks/edge_new.hxx"
#include "./neural_networks/neural_network.hxx"

#include "./util/tao_random.hxx"

#ifdef _BOINC_
#ifdef _WIN32
    #include "boinc_win.h"
    #include "str_util.h"
#endif

    #include "diagnostics.h"
    #include "util.h"
    #include "filesys.h"
    #include "boinc_api.h"
    #include "mfile.h"
#endif


//JSON11 parser
#include "rapidjson/document.h"
using namespace rapidjson;

int my_isinf(double x) {
    return !isnan(x) && isnan(x - x);
}

int my_isnan(double x) {
    return x != x;
}


double linear_activation_function(double input) {
    return input;
}

double relu_activation_function(double input) {
    return fmax(0,input);
}


double sigmoid_activation_function(double input) {
    return 1.0 / (1.0 + exp(-input));
}

double tanh_activation_function(double input) {
    return tanh(input);
}

double linear_derivative(double input) {
    return 1.0;
}

double relu_derivative(double input) {
    return fmax(0,input);
}

double sigmoid_derivative(double input) {
    return input * (1 - input);
}

double tanh_derivative(double input) {
    return 1 - (tanh(input) * tanh(input));
}



Neuron::Neuron(uint32_t _depth, uint32_t _layer, uint32_t _node) : depth(_depth), layer(_layer), node(_node), identifier(""), bias(0.0), value(0.0), error(0.0) {
}

Neuron::~Neuron() {
    //NeuralNetwork should clean up all the neuron edges
}

void Neuron::connect_forward(EdgeNew *edge) {
    forward_edges.push_back(edge);
}

void Neuron::connect_backward(EdgeNew *edge) {
    backward_edges.push_back(edge);
}

string Neuron::json() const {
    ostringstream oss;
    oss << "{ "
        << "\"depth\" : " << depth << ", "
        << "\"layer\" : " << layer << ", "
        << "\"node\" : " << node << ", "
        << "\"identifier\" : \"" << identifier << "\", "
        << "\"bias\" : " << bias << ", "
        << "\"value\" : " << value << ", "
        << "\"error\" : " << error 
        << "}";

    return oss.str();
}

ostream& operator<< (ostream& out, const Neuron &neuron) {
    out << neuron.json();
    return out;
}

ostream& operator<< (ostream& out, const Neuron *neuron) {
    out << neuron->json();
    return out;
}


void NeuralNetwork::initialize_nodes() {
    //initialize all the possible nodes to null
    for (uint32_t depth = 0; depth < recurrent_depth; depth++) {
        nodes.push_back( vector< vector<Neuron*> >() );

        uint32_t layer = 0;
        nodes[depth].push_back( vector<Neuron*>() );

        for (uint32_t node = 0; node < n_input_nodes; node++) {
            nodes[depth][layer].push_back( NULL );
        }

        layer = 1;
        for (; layer < n_hidden_layers + 1; layer++) {
            nodes[depth].push_back( vector<Neuron*>() );
            for (uint32_t node = 0; node < n_hidden_nodes; node++) {
                nodes[depth][layer].push_back( NULL );
            }
        }

        //no recurrent layer on the output layer
        if (depth == 0) {
            nodes[depth].push_back( vector<Neuron*>() );
            for (uint32_t node = 0; node < n_output_nodes; node++) {
                nodes[depth][layer].push_back( NULL );
            }
        }
    }
}

void NeuralNetwork::set_activation_functions(string activation_function_str) {
    if (activation_function_str.compare("sigmoid_softmax") == 0) {
        activation_function = sigmoid_activation_function;
        derivative_function = sigmoid_derivative;
    } else if (activation_function_str.compare("tanh_softmax") == 0) {
        activation_function = tanh_activation_function;
        derivative_function = tanh_derivative;
    } else if (activation_function_str.compare("linear") == 0) {
        activation_function = linear_activation_function;
        derivative_function = linear_derivative;
    } else {
        cerr << "Unknown activation function specified: '" << activation_function_str << "'." << endl;
        cerr << "Options are: " << endl;
        cerr << "    sigmoid_softmax    -- sigmoid on hidden nodes, softmax on output layer" << endl;
        cerr << "    tanh_softmax       -- tanh on hidden nodes, softmax on output layer" << endl;
        cerr << "    linear             -- linear on hidden and output layers" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }
}

void boinc_assert(bool result, string error_message) {
    if (!result) {
        cerr << "ERROR: " << error_message << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }
}

NeuralNetwork::NeuralNetwork(string json_filename) {
    kahan_summation = false;
    batch_update = false;

    //read entire file from filename
    ifstream ifs(json_filename);
    string content( (istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()) );

    rapidjson::Document json_document;
    if (json_document.Parse(content.c_str()).HasParseError()) {
        boinc_assert(false, string("parsing neural network JSON file '") + json_filename +  string("'."));
    }

    cerr << "successfully parsed '" << json_filename << "'" << endl;

    boinc_assert(json_document.IsObject(), string("JSON document was not an object."));
    boinc_assert(json_document.HasMember("recurrent_depth"), string("JSON document did not have recurrent_depth field."));
    boinc_assert(json_document.HasMember("n_input_nodes"), string("JSON document did not have n_input_nodes field."));
    boinc_assert(json_document.HasMember("n_hidden_layers"), string("JSON document did not have n_hidden_layers field."));
    boinc_assert(json_document.HasMember("n_hidden_nodes"), string("JSON document did not have n_hidden_nodes field."));
    boinc_assert(json_document.HasMember("n_output_nodes"), string("JSON document did not have n_output_nodes field."));

    recurrent_depth = json_document["recurrent_depth"].GetInt();
    n_input_nodes   = json_document["n_input_nodes"].GetInt();
    n_hidden_layers = json_document["n_hidden_layers"].GetInt();
    n_hidden_nodes  = json_document["n_hidden_nodes"].GetInt();
    n_output_nodes  = json_document["n_output_nodes"].GetInt();

    cerr << "recurrent_depth: " << recurrent_depth << endl;
    cerr << "n_input_nodes: " << n_input_nodes << endl;
    cerr << "n_hidden_layers: " << n_hidden_layers << endl;
    cerr << "n_hidden_nodes: " << n_hidden_nodes << endl;
    cerr << "n_output_nodes: " << n_output_nodes << endl;

    initialize_nodes();

    boinc_assert(json_document.HasMember("edges"), "JSON document did not have edges field.");
    boinc_assert(json_document.HasMember("recurrent_edges"), "JSON document did not have recurrent_edges field.");
    boinc_assert(json_document["edges"].IsArray(), "JSON document field 'edges' was not array.");
    boinc_assert(json_document["recurrent_edges"].IsArray(), "JSON document field 'recurrent_edges' was not array.");


    vector<EdgeNew> _edges, _recurrent_edges;
    const Value& a1 = json_document["edges"];
    for (uint32_t i = 0; i < a1.Size(); i++) {
        //cout << nn_content["edges"].array_items()[i].dump() << endl;

        const Value &edge = a1[i];
        ostringstream oss;
        oss << "JSON docuemnt field edges[" << i << "] was not an Object.";
        boinc_assert(edge.IsObject(), oss.str());

        _edges.push_back(EdgeNew(edge["src_depth"].GetInt(),
                                edge["src_layer"].GetInt(),
                                edge["src_node"].GetInt(),
                                edge["dst_depth"].GetInt(),
                                edge["dst_layer"].GetInt(),
                                edge["dst_node"].GetInt(),
                                edge["weight"].GetDouble()));

        cout << "pushed back edge: " << _edges.back() << endl;
    }

    const Value& a2 = json_document["recurrent_edges"];
    for (uint32_t i = 0; i < a2.Size(); i++) {
        //cout << nn_content["recurrent_edges"].array_items()[i].dump() << endl;

        const Value &recurrent_edge = a2[i];
        ostringstream oss;
        oss << "JSON docuemnt field recurrent_edges[" << i << "] was not an Object.";
        boinc_assert(recurrent_edge.IsObject(), oss.str());

        _recurrent_edges.push_back(EdgeNew(recurrent_edge["src_depth"].GetInt(),
                                recurrent_edge["src_layer"].GetInt(),
                                recurrent_edge["src_node"].GetInt(),
                                recurrent_edge["dst_depth"].GetInt(),
                                recurrent_edge["dst_layer"].GetInt(),
                                recurrent_edge["dst_node"].GetInt(),
                                recurrent_edge["weight"].GetDouble()));

        cout << "pushed back recurrent edge: " << _recurrent_edges.back() << endl;
    }

    set_edges(_edges, _recurrent_edges);
    cerr << "set " << edges.size() << " edges and " << recurrent_edges.size() << " recurrent edges." << endl;

    boinc_assert(json_document.HasMember("activation_function"), "JSON document did not have activation_function field.");
    string activation_function_str = json_document["activation_function"].GetString();
    set_activation_functions(activation_function_str);
    cerr << "set '" << activation_function_str << "' activation and derivative functions" << endl;
}

NeuralNetwork::NeuralNetwork(uint32_t _recurrent_depth, uint32_t _n_input_nodes, uint32_t _n_hidden_layers, uint32_t _n_hidden_nodes, uint32_t _n_output_nodes, string _activation_function_str) : recurrent_depth(_recurrent_depth), n_input_nodes(_n_input_nodes), n_hidden_layers(_n_hidden_layers), n_hidden_nodes(_n_hidden_nodes), n_output_nodes(_n_output_nodes), activation_function_str(_activation_function_str) {

    kahan_summation = false;
    batch_update = false;

    initialize_nodes();
    set_activation_functions(activation_function_str);

    /*
    for (uint32_t i = 0; i < nodes.size(); i++) {
        cout << "depth[" << i << "/" << nodes.size() << "]" << endl;
        for (uint32_t j = 0; j < nodes[i].size(); j++) {
            cout << "    layer[" << j << "/" << nodes[i].size() << "] size: " << nodes[i][j].size() << endl;
        }
    }
    */
}

NeuralNetwork::~NeuralNetwork() {
    reset();
}

void NeuralNetwork::use_kahan_summation(bool val) {
    kahan_summation = val;
}

void NeuralNetwork::use_batch_update(bool val) {
    batch_update = val;
}

void NeuralNetwork::reset() {
    while (edges.size() > 0) {

        EdgeNew *edge = edges.back();
        edges.pop_back();
        delete edge;
    }

    while (recurrent_edges.size() > 0) {
        EdgeNew *edge = recurrent_edges.back();
        recurrent_edges.pop_back();
        delete edge;
    }

    for (uint32_t depth = 0; depth < nodes.size(); depth++) {
        for (uint32_t layer = 0; layer < nodes[depth].size(); layer++) {
            for (uint32_t node = 0; node < nodes[depth][layer].size(); node++) {
                delete nodes[depth][layer][node];
                nodes[depth][layer][node] = NULL;
            }
        }
    }

    linear_nodes.clear();
}


void NeuralNetwork::set_training_data(uint32_t n_examples, uint32_t input_size, double **_inputs, uint32_t output_size, double **_outputs) {
    inputs = vector< vector<double> >(n_examples, vector<double>(input_size, 0.0));
    outputs = vector< vector<double> >(n_examples, vector<double>(output_size, 0.0));

    /*
    cout << "setting training data, "
         << " input size[" << inputs.size() << "][" << inputs[0].size() << "], "
         << " output size[" << outputs.size() << "][" << outputs[0].size() << "]" << endl;
    */

    for (uint32_t i = 0; i < n_examples; i++) {
        for (uint32_t j = 0; j < input_size; j++) {
            inputs[i][j] = _inputs[i][j];
        }

        for (uint32_t j = 0; j < output_size; j++) {
            outputs[i][j] = _outputs[i][j];
        }
    }
}

void NeuralNetwork::set_training_data(const vector< vector<double> > &_inputs, const vector< vector<double> > &_outputs) {
    inputs = _inputs;
    outputs = _outputs;
}

void NeuralNetwork::set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges) {
    reset();

    for (uint32_t i = 0; i < _edges.size(); i++) {
        edges.push_back(new EdgeNew(_edges[i].src_depth, _edges[i].src_layer, _edges[i].src_node, _edges[i].dst_depth, _edges[i].dst_layer, _edges[i].dst_node, _edges[i].weight));
    }

    for (uint32_t i = 0; i < _recurrent_edges.size(); i++) {
        recurrent_edges.push_back(new EdgeNew(_recurrent_edges[i].src_depth, _recurrent_edges[i].src_layer, _recurrent_edges[i].src_node, _recurrent_edges[i].dst_depth, _recurrent_edges[i].dst_layer, _recurrent_edges[i].dst_node, _recurrent_edges[i].weight));
    }

    uint32_t src_depth, src_layer, src_node;
    uint32_t dst_depth, dst_layer, dst_node;

    Neuron *src_neuron, *dst_neuron;

    //cout << "setting edges: " << endl;
    for (uint32_t edge = 0; edge < edges.size(); edge++) {
        //cout << "setting edge: " << edges[edge] << endl;

        src_depth = edges[edge]->src_depth;
        src_layer = edges[edge]->src_layer;
        src_node = edges[edge]->src_node;

        //cout << "setting source neuron" << endl;
        src_neuron = NULL;
        if (nodes[src_depth][src_layer][src_node] == NULL) {
            src_neuron = new Neuron(src_depth, src_layer, src_node);
            nodes[src_depth][src_layer][src_node] = src_neuron;
        } else {
            src_neuron = nodes[src_depth][src_layer][src_node];
        }

        dst_depth = edges[edge]->dst_depth;
        dst_layer = edges[edge]->dst_layer;
        dst_node = edges[edge]->dst_node;

        //cout << "setting dst neuron" << endl;
        dst_neuron = NULL;
        if (nodes[dst_depth][dst_layer][dst_node] == NULL) {
            dst_neuron = new Neuron(dst_depth, dst_layer, dst_node);
            nodes[dst_depth][dst_layer][dst_node] = dst_neuron;
        } else {
            dst_neuron = nodes[dst_depth][dst_layer][dst_node];
        }

        //cout << "connecting forward" << endl;
        src_neuron->connect_forward(edges[edge]);

        //cout << "connecting backward" << endl;
        dst_neuron->connect_backward(edges[edge]);
    }
    //cout << "set edges." << endl;

    //cout << "setting recurrent edges: " << endl;
    for (uint32_t edge = 0; edge < recurrent_edges.size(); edge++) {
        //cout << "setting edge: " << recurrent_edges[edge] << endl;

        src_depth = recurrent_edges[edge]->src_depth;
        src_layer = recurrent_edges[edge]->src_layer;
        src_node = recurrent_edges[edge]->src_node;

        //cout << "setting source neuron" << endl;
        src_neuron = NULL;
        if (nodes[src_depth][src_layer][src_node] == NULL) {
            src_neuron = new Neuron(src_depth, src_layer, src_node);
            nodes[src_depth][src_layer][src_node] = src_neuron;
        } else {
            src_neuron = nodes[src_depth][src_layer][src_node];
        }

        dst_depth = recurrent_edges[edge]->dst_depth;
        dst_layer = recurrent_edges[edge]->dst_layer;
        dst_node = recurrent_edges[edge]->dst_node;

        //cout << "setting dst neuron" << endl;
        dst_neuron = NULL;
        if (nodes[dst_depth][dst_layer][dst_node] == NULL) {
            dst_neuron = new Neuron(dst_depth, dst_layer, dst_node);
            nodes[dst_depth][dst_layer][dst_node] = dst_neuron;
        } else {
            dst_neuron = nodes[dst_depth][dst_layer][dst_node];
        }

        //cout << "connecting forward" << endl;
        src_neuron->connect_forward(recurrent_edges[edge]);

        //cout << "connecting backward" << endl;
        dst_neuron->connect_backward(recurrent_edges[edge]);
    }
    //cout << "set recurrent edges." << endl;

    active_input_nodes = 0;
    for (uint32_t depth = 0; depth < nodes.size(); depth++) {
        for (uint32_t layer = 0; layer < nodes[depth].size(); layer++) {
            for (uint32_t node = 0; node < nodes[depth][layer].size(); node++) {
                if (nodes[depth][layer][node] != NULL) {
                    if (layer == 0) active_input_nodes++;
                    linear_nodes.push_back(nodes[depth][layer][node]);
                }
            }
        }
    }

    output_layer = nodes[0].size() - 1;
}

void NeuralNetwork::set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges, const vector<string> &input_labels, const vector<string> &output_labels) {
    set_edges(_edges, _recurrent_edges);

    if (nodes[0][0].size() != input_labels.size()) {
        cerr << "ERROR on '" << __FILE__ << ":" << __LINE__ << "':" << endl;
        cerr << "\tinput label size (" << input_labels.size() << ") does not match nodes[0][0].size (" << nodes[0][0].size() << ")" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    if (nodes[0][nodes[0].size() - 1].size() != output_labels.size()) {
        cerr << "ERROR on '" << __FILE__ << ":" << __LINE__ << "':" << endl;
        cerr << "\toutput label size (" << output_labels.size() << ") does not match nodes[0][nodes.size() - 1].size (" << nodes[0][nodes[0].size() - 1].size() << ")" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    for (uint32_t i = 0; i < input_labels.size(); i++) {
        if (nodes[0][0][i] != NULL) {
            nodes[0][0][i]->identifier = input_labels[i];
        }
    }

    for (uint32_t i = 0; i < output_labels.size(); i++) {
        if (nodes[0][nodes[0].size() - 1][i] != NULL) {
            nodes[0][nodes[0].size() - 1][i]->identifier = output_labels[i];
        }
    }

}

string NeuralNetwork::json() {
    ostringstream oss;

    oss << "{" << endl;
    oss << "    \"recurrent_depth\" : " << recurrent_depth << "," << endl;
    oss << "    \"n_input_nodes\" : " << n_input_nodes << "," << endl;
    oss << "    \"n_hidden_layers\" : " << n_hidden_layers << "," << endl;
    oss << "    \"n_hidden_nodes\" : " << n_hidden_nodes << "," << endl;
    oss << "    \"n_output_nodes\" : " << n_output_nodes << "," << endl;
    oss << "    \"activation_function\" : \"" << activation_function_str << "\"," << endl;
    oss << "    \"nodes\" : [" << endl;

    bool printed_previously = false;
    for (uint32_t i = 0; i < nodes.size(); i++) {
        for (uint32_t j = 0; j < nodes[i].size(); j++) {
            for (uint32_t k = 0; k < nodes[i][j].size(); k++) {

                if (nodes[i][j][k] != NULL) {
                    if (printed_previously) oss << "," << endl << "        " << nodes[i][j][k]->json();
                    else oss << "        " << nodes[i][j][k]->json();
                    printed_previously = true;
                }
            }
        }
    }
    oss << endl;
    oss << "    ]," << endl;

    oss << "    \"edges\" : [" << endl;
    for (uint32_t i = 0; i < edges.size(); i++) {
        oss << "        " << edges[i]->json();
        if (i != edges.size() - 1) oss << ", " << endl;
        else oss << endl;
    }
    oss << "    ]," << endl;

    oss << "    \"recurrent_edges\" : [" << endl;
    for (uint32_t i = 0; i < recurrent_edges.size(); i++) {
        oss << "        " << recurrent_edges[i]->json();
        if (i != recurrent_edges.size() - 1) oss << ", " << endl;
        else oss << endl;
    }
    oss << "    ]" << endl;
    oss << "}" << endl;

    return oss.str();
}

void NeuralNetwork::write_to_file(string json_filename) {
    ofstream outfile(json_filename);

    if (outfile.good()) {
        outfile << json();
    } else {
        cerr << "ERROR on '" << __FILE__ << ":" << __LINE__ << "':" << endl;
        cerr << "\tcould not write neural network to output file '" << json_filename << "'" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    outfile.close();
}

uint32_t NeuralNetwork::get_parameter_size() {
    //input nodes have no bias
    return linear_nodes.size() + edges.size() - active_input_nodes;
}

void NeuralNetwork::set_weights(const vector<double> &weights) {
    if (weights.size() != get_parameter_size()) {
        cerr << "ERROR on '" << __FILE__ << ":" << __LINE__ << "':" << endl;
        cerr << "\tnumber of weights to evaluate neural network with (" << weights.size() << ") != number of nodes (" << linear_nodes.size() << ") + number of edges (" << edges.size() << ")" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    /*
    cout << "weights:";
    for (uint32_t i = 0; i < weights.size(); i++) {
        cout << " " << weights[i];
    }   
    cout << endl;
    */

    uint32_t current = 0;

    for (uint32_t node = 0; node < linear_nodes.size(); node++) {
        linear_nodes[node]->value = 0;

        //input nodes have no bias
        if (linear_nodes[node]->layer != 0) {
            linear_nodes[node]->bias = weights[current];
            current++;
        }

    }

    for (uint32_t edge = 0; edge < edges.size(); edge++) {
        edges[edge]->weight = weights[current];
        current++;
    }

    if (current != weights.size()) {
        cerr << "ERROR: current (" << current << ") != weights.size() (" << weights.size() << ")" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    //recurrent edges have no weight
    //TODO: try recurrent edge weights?
}

void NeuralNetwork::evaluate_at(uint32_t example) {

    //set initial bias and input values
    for (uint32_t node = 0; node < linear_nodes.size(); node++) {
        if (linear_nodes[node]->depth == 0) {
            if (linear_nodes[node]->layer == 0) {
                linear_nodes[node]->value = inputs[example][linear_nodes[node]->node];
            } else {
                //reset non-recurrent node values equal to bias
                linear_nodes[node]->value = linear_nodes[node]->bias;
            }
        } else {
            //add bias to recurrent node values
            linear_nodes[node]->value += linear_nodes[node]->bias;
        }
        
        //cout << "initialized " << linear_nodes[node] << endl;
    }

    //cout << "output node value: " << nodes[0][output_layer][0]->value << endl;

    //propogate values forward
    Neuron *current, *target;
    uint32_t dst_depth, dst_layer, dst_node;
    for (uint32_t node = 0; node < linear_nodes.size(); node++) {
        current = linear_nodes[node];


        //cout << current << ", current->value: " << current->value << ", after activation function: " << activation_function(current->value) << endl;
        current->value = activation_function(current->value);

        if (current->value > 10000) current->value = 10000;
        if (current->value < -10000) current->value = -10000;

        for (uint32_t next = 0; next < current->forward_edges.size(); next++) {
            dst_depth = current->forward_edges[next]->dst_depth;
            dst_layer = current->forward_edges[next]->dst_layer;
            dst_node = current->forward_edges[next]->dst_node;

            target = nodes[dst_depth][dst_layer][dst_node];
            
            //cout << "setting target->value " << target->value << " += " << current->value << " * " << current->forward_edges[next]->weight << endl;

            target->value += current->value * current->forward_edges[next]->weight;
        }
    }
    //cout << "output node value: " << nodes[0][output_layer][0]->value << endl;
}



void NeuralNetwork::evaluate_at_softmax(uint32_t example) {

    //set initial bias and input values
    for (uint32_t node = 0; node < linear_nodes.size(); node++) {
        if (linear_nodes[node]->depth == 0) {
            if (linear_nodes[node]->layer == 0) {
                linear_nodes[node]->value = inputs[example][linear_nodes[node]->node];
            } else {
                //reset non-recurrent node values equal to bias
                linear_nodes[node]->value = linear_nodes[node]->bias;
            }
        } else {
            //add bias to recurrent node values
            linear_nodes[node]->value += linear_nodes[node]->bias;
        }
        //            cout << linear_nodes[node] << endl;
    }

    //        cout << "output node value: " << nodes[0][output_layer][0]->value << endl;

    //propogate values forward
    Neuron *current, *target;
    uint32_t dst_depth, dst_layer, dst_node;
    for (uint32_t node = 0; node < linear_nodes.size(); node++) {
        current = linear_nodes[node];

        double previous_value = current->value;

        if (current->layer == output_layer) {
            if (current->value > 10000) current->value = 10000.0;
            if (current->value < -10000) current->value = -10000.0;
        }

        //cout << "forward propagating from: " << current << endl;

        if (my_isinf(current->value)) {
            cout << "current: " << current << endl;
            cout << "INF from activation function(" << previous_value << ")!" << endl;
            cout << "current->value > 0? " << (current->value > 0) << endl;
            cout << "current->value < 0? " << (current->value > 0) << endl;
#ifdef _BOINC_
            boinc_finish(1);
#endif
            exit(1);
        }

        if (my_isnan(current->value)) {
            cout << "current: " << current << endl;
            cout << "NAN from activation function(" << previous_value << ")!" << endl;
#ifdef _BOINC_
            boinc_finish(1);
#endif
            exit(1);
        }

        if (current->layer == output_layer) {
            //on the output layer, make the output exp(output) for softmax layer
            //double previous = current->value;
            current->value = exp(current->value);

            //cout << "on output layer, node: " << current << ", value before output activation function() = " << previous << endl;


        } else if (current->layer > 0) {
            //cout << "current->value: " << current->value << ", after activation function: " << activation_function(current->value) << endl;

            current->value = activation_function(current->value);
        }

        for (uint32_t next = 0; next < current->forward_edges.size(); next++) {
            dst_depth = current->forward_edges[next]->dst_depth;
            dst_layer = current->forward_edges[next]->dst_layer;
            dst_node = current->forward_edges[next]->dst_node;

            target = nodes[dst_depth][dst_layer][dst_node];

            //cout << "    propagating to: " << target << endl;

            target->value += current->value * current->forward_edges[next]->weight;
        }
    }
}


void NeuralNetwork::update_recurrent_nodes() {

    uint32_t dst_depth, dst_layer, dst_node;
    uint32_t src_depth, src_layer, src_node;

    //copy values to recurrent neurons, starting at the deepest layer any working up
    for (int32_t i = recurrent_edges.size() - 1; i >= 0; i--) {
        dst_depth = recurrent_edges[i]->dst_depth;
        dst_layer = recurrent_edges[i]->dst_layer;
        dst_node = recurrent_edges[i]->dst_node;

        src_depth = recurrent_edges[i]->src_depth;
        src_layer = recurrent_edges[i]->src_layer;
        src_node = recurrent_edges[i]->src_node;

        nodes[dst_depth][dst_layer][dst_node]->value = nodes[src_depth][src_layer][src_node]->value;

        //cout << "set recurrent node: " << nodes[dst_depth][dst_layer][dst_node]->value << endl;

        if (nodes[dst_depth][dst_layer][dst_node]->value > 10000.00) {
            nodes[dst_depth][dst_layer][dst_node]->value = 10000.00;
            //cout << "recurrent node too big!" << endl;
        }

        if (nodes[dst_depth][dst_layer][dst_node]->value < -10000.00) {
            nodes[dst_depth][dst_layer][dst_node]->value = -10000.00;
            //cout << "recurrent node too small!" << endl;
        }

        //            cout << "set recurrent node: " << nodes[recurrent_edges.at(i]->dst_depth)[recurrent_edges.at(i]->dst_layer)[recurrent_edges.at(i]->dst_node) << endl;
    }
}

double NeuralNetwork::objective_function(const vector<double> &weights) {
    set_weights(weights);

    double error = 0.0;
    for (uint32_t example = 0; example < inputs.size(); example++) {
        evaluate_at(example);

        //backward propagation of errors
        double node_error;
        Neuron *current;
        for (int32_t node = 0; node < nodes[0][output_layer].size(); node++) {
            current = nodes[0][output_layer][node];

            node_error = outputs[example][current->node] - current->value;

            error += fabs(node_error);

            //error += 0.5 * (node_error * node_error);
        }

        update_recurrent_nodes();
    }

    error /= (-1.0 * inputs.size());
//    cout << "error was: " << error << endl;

    return error;
}

void NeuralNetwork::print_predictions(ofstream &outfile) {
    outfile << "#actual,predicted" << endl;

    double error = 0.0;
    for (uint32_t example = 0; example < inputs.size(); example++) {
        evaluate_at(example);

        //backward propagation of errors
        double node_error;
        Neuron *current;
        for (int32_t node = 0; node < nodes[0][output_layer].size(); node++) {
            current = nodes[0][output_layer][node];

            node_error = outputs[example][current->node] - current->value;

            error += fabs(node_error);

            outfile << outputs[example][current->node] << "," << current->value << endl;
            //error += 0.5 * (node_error * node_error);
        }

        update_recurrent_nodes();
    }

    error /= (-1.0 * inputs.size());
    outfile << "#final error was: " << error << endl;
}


double NeuralNetwork::backpropagation_time_series(const vector<double> &starting_weights, double learning_rate, uint32_t max_iterations) {
    set_weights(starting_weights);

    double previous_error = -numeric_limits<double>::max();
    double error;
    double mean_average_error; 
    double best_mean_average_error = -numeric_limits<double>::max();
    int best_iteration = 0;

    for (uint32_t iteration = 0; iteration < max_iterations; iteration++) {
        error = 0.0;
        mean_average_error = 0.0;
        uint32_t dst_depth, dst_layer, dst_node;
        for (uint32_t example = 0; example < inputs.size(); example++) {
            evaluate_at(example);

            if (batch_update || kahan_summation) {
                for (uint32_t edge = 0; edge < edges.size(); edge++) {
                    edges[edge]->weight_carry = 0.0;
                    edges[edge]->weight_sum = 0.0;
                }
            }

            //backward propagation of errors
            double node_error;
            Neuron *current;
            for (int32_t node = linear_nodes.size() - 1; node >= 0; node--) {
                current = linear_nodes[node];

                //cout << "output layer: " << output_layer << ", current: " << current << endl;

                if (current->layer == output_layer) {
                    node_error = outputs[example][current->node] - current->value;

                    //cout << "example " << example << ": actual (" << outputs[example][current->node] << "), predicted (" << current->value << "), error: " << node_error << endl;

                    mean_average_error += fabs(node_error);
                    error += 0.5 * (node_error * node_error);

                    if (isnan(error)) {
                        cout << "error became NAN on example " << example << " and iteration " << iteration << " from adding " << (0.5 * (node_error * node_error)) << ", node error: " << node_error << endl;
                        cout << "outputs[" << example << "][" << current->node << "]: " << outputs[example][current->node] << ", current->value: " << current->value << endl;
                        cout << "weights into current: " << endl;

                        double current_sum = 0.0;
                        for (uint32_t i = 0; i < current->backward_edges.size(); i++) {
                            EdgeNew *back_edge = current->backward_edges[i];
                            Neuron *current = nodes[back_edge->src_depth][back_edge->src_layer][back_edge->src_node];
                            current_sum += current->value;

                            cout << back_edge << " " << current << ", current sum: " << current_sum << endl;
                        }
                        cout << "sum recalculated: " << current_sum << ", exp(sum)" << exp(current_sum) << endl;


                        ostringstream weights;
                        weights << "starting weights:";
                        for (uint32_t i = 0; i < starting_weights.size(); i++) {
                            weights << " " << starting_weights[i];
                        }
                        cout << weights.str() << endl;
                    }
                } else {
                    node_error = 0.0;
                    for (uint32_t next = 0; next < current->forward_edges.size(); next++) {
                        dst_depth = current->forward_edges[next]->dst_depth;
                        dst_layer = current->forward_edges[next]->dst_layer;
                        dst_node = current->forward_edges[next]->dst_node;


                        double weight = current->forward_edges[next]->weight;
                        if (isnan(weight) || isinf(weight)) {
                            cout << "NAN/INF on edge: " << current->forward_edges[next]->weight << endl;
                        }

                        double error = nodes[dst_depth][dst_layer][dst_node]->error;
                        if (isnan(error) || isinf(error)) {
                            cout << "NAN/INF on node: " << nodes[dst_depth][dst_layer][dst_node] << endl;
                        }

                        node_error += current->forward_edges[next]->weight * nodes[dst_depth][dst_layer][dst_node]->error;

                        if (kahan_summation) {
                            double weight_delta = nodes[dst_depth][dst_layer][dst_node]->error * current->value;

                            double y = weight_delta - current->forward_edges[next]->weight_carry;
                            double t = current->forward_edges[next]->weight_sum + y;

                            current->forward_edges[next]->weight_carry = (t - current->forward_edges[next]->weight_sum) - y;
                            current->forward_edges[next]->weight_sum = t;
                        } else if (batch_update) {
                            current->forward_edges[next]->weight_sum += nodes[dst_depth][dst_layer][dst_node]->error * current->value;
                        } else {
                            double weight_delta = learning_rate * nodes[dst_depth][dst_layer][dst_node]->error * current->value;

                            if (weight_delta < -0.0001) weight_delta = -0.0001;
                            if (weight_delta > 0.0001) weight_delta =  0.0001;

                            current->forward_edges[next]->weight += weight_delta;
                        }
                    }
                }

                if (kahan_summation || batch_update) {
                    for (uint32_t edge = 0; edge < edges.size(); edge++) {
                        //edges[edge]->weight += learning_rate * (edges[edge]->weight_sum / inputs.size());

                        double weight_delta = learning_rate * edges[edge]->weight_sum;

                        if (weight_delta < -0.0001) weight_delta = -0.0001;
                        if (weight_delta > 0.0001) weight_delta =  0.0001;

                        edges[edge]->weight += weight_delta;
                    }
                }

                current->error = node_error * derivative_function(current->value);

                current->bias += learning_rate * current->error;

                //cout << current << endl;
            }

            update_recurrent_nodes();
        }

        mean_average_error /= (-1.0 * inputs.size());
        error /= (-1.0 * inputs.size());

        if (iteration % 10 == 0) {
            //cout << "[iteration: " << iteration << "] error was: " << error << ", mean_average_error: " << mean_average_error << endl;
        }

        if (error < previous_error) {
            //cout << "[iteration: " << iteration << "] breaking because error (" << error << ") < previous error (" << previous_error << ")" << endl;
            //return mean_average_error;

            //learning_rate *= 0.1;
            //cout << "decrementing learning rate to: " << learning_rate << endl;
        }
        previous_error = error;

        if (mean_average_error > best_mean_average_error) {
            best_mean_average_error = mean_average_error;
            best_iteration = iteration;
        }

        //break if we're not getting anywhere
        if (iteration - best_iteration > 20) break;
    }

    cout << "best fitness " << best_mean_average_error << " found on iteration: " << best_iteration << endl;

    return best_mean_average_error;
}

double NeuralNetwork::backpropagation_stochastic(const vector<double> &starting_weights, double learning_rate, uint32_t max_iterations, TaoRandom &generator) {
    set_weights(starting_weights);

    double previous_error = -numeric_limits<double>::max();
    double error;
    uint32_t misclassified_examples = 0;

    vector<uint32_t> shuffled_examples(inputs.size());
    for (uint32_t i = 0; i < inputs.size(); i++) {
        shuffled_examples[i] = i;
    }

    uint32_t iteration = 0;

#ifdef _BOINC_
    read_checkpoint("checkpoint.bin", generator, shuffled_examples, iteration);
#endif

    for (; iteration < max_iterations; iteration++) {
#ifdef _BOINC_
        boinc_fraction_done((double)iteration / (double)max_iterations);
        //ostringstream checkpoint_oss;
        //checkpoint_oss << "checkpoint_" << iteration << ".bin";
        //write_checkpoint(checkpoint_oss.str(), generator, shuffled_examples, iteration);

        if (boinc_time_to_checkpoint()) {
            write_checkpoint("checkpoint.bin", generator, shuffled_examples, iteration);
            boinc_checkpoint_completed();
        }
#endif


        error = 0.0;
        misclassified_examples = 0;
        uint32_t dst_depth, dst_layer, dst_node;

        random_shuffle(shuffled_examples.begin(), shuffled_examples.end(), generator);

        /*
        cout << "n random numbers generated: " << generator.get_n_generated() << endl;
        cout << "first 5 shuffled examples: " << endl;
        for (uint32_t i = 0; i < 5; i++) {
            cout << "    " << shuffled_examples[i] << endl;
        }
        */

        double min_weight_delta = numeric_limits<double>::max();
        double avg_weight_delta = 0.0;
        double max_weight_delta = -numeric_limits<double>::max();
        uint64_t avg_weight_delta_count = 0;

        for (uint32_t example = 0; example < inputs.size(); example++) {
            //cout << "evaluating example: " << shuffled_examples[example] << endl;
            //if (example % 1000 == 0) cout << "evaluating " << example << " of " << inputs.size() << endl;

            evaluate_at_softmax(shuffled_examples[example]);

            double output_layer_sum = 0.0;
            double max_output = nodes[0][output_layer][0]->value;
            double current_output;
            uint32_t max_output_label = 0;
            for (uint32_t i = 0; i < nodes[0][output_layer].size(); i++) {
                if (nodes[0][output_layer][i] != NULL) {
                    current_output = nodes[0][output_layer][i]->value;
                    output_layer_sum += current_output;

                    if (i > 0) {
                        if (max_output < current_output) {
                            max_output_label = i;
                            max_output = current_output;
                        }
                    }
                }
            }

            //test to see if all the output nodes are the same value,
            //in this case, all are wrong
            double test;
            for (uint32_t i = 0; i < nodes[0][output_layer].size(); i++) {
                if (nodes[0][output_layer][i] != NULL) {
                    test = nodes[0][output_layer][i]->value;
                }
            }

            bool all_equal = true;
            for (uint32_t i = 0; i < nodes[0][output_layer].size(); i++) {
                if (nodes[0][output_layer][i] != NULL) {
                    if (test != nodes[0][output_layer][i]->value) {
                        all_equal = false;
                        break;
                    }
                }
            }

            if (all_equal) {
                max_output_label = -1;

                for (uint32_t i = 0; i < nodes[0][output_layer].size(); i++) {
                    if (nodes[0][output_layer][i] != NULL) {
                        //nodes[0][output_layer][i]->value = 0.0000001;
                        nodes[0][output_layer][i]->value = 0;
                    }
                }
            }


            if (output_layer_sum == 0) {
                cout << "ERROR: output layer sum == 0" << endl;

                for (uint32_t i = 0; i < nodes[0][output_layer].size(); i++) {
                    if (nodes[0][output_layer][i] != NULL) {
                        Neuron *current = nodes[0][output_layer][i];
                        current_output = current->value;

                        cout << "    nodes[0][" << output_layer << "][" << i << "]->value: " << current_output << endl;

                        cout << "        outputs[example][current->node]: " << outputs[example][current->node] << ", current->value: " << current->value << endl;
                        cout << "        weights into current: " << endl;

                        double current_sum = current->bias;
                        for (uint32_t i = 0; i < current->backward_edges.size(); i++) {
                            EdgeNew *back_edge = current->backward_edges[i];
                            Neuron *current = nodes[back_edge->src_depth][back_edge->src_layer][back_edge->src_node];
                            current_sum += current->value;

                            cout << "            " << back_edge << " " << current << ", current sum: " << current_sum << endl;
                        }
                        cout << "    sum recalculated: " << current_sum << ", exp(sum): " << exp(current_sum) << endl;

                    }
                }

#ifdef _BOINC_
                boinc_finish(1);
#endif
                exit(1);
            }


            if (max_output_label != outputs[shuffled_examples[example]][0]) {
                misclassified_examples++;
            }

            //backward propagation of errors
            double node_error;
            double previous_value;
            Neuron *current;
            for (int32_t node = linear_nodes.size() - 1; node >= 0; node--) {
                current = linear_nodes[node];

                if (current->layer == output_layer) {
                    //softmax layer
                    previous_value = current->value;
                    current->value = current->value / output_layer_sum;

                    if (outputs[shuffled_examples[example]][0] == current->node) {
                        node_error = 1.0 - current->value;

                        if (current->value == 0) {
                            error += 1000.0;
                        } else {
                            error += -log(current->value);
                        }
                    } else {
                        node_error = 0.0 - current->value;
                        error += 0;
                    }

                    /*
                    if (all_equal) {
                        cout << "output layer[" << current->node << "]: node error: " << node_error << ", expected class: " << outputs[shuffled_examples[example]][0] << endl;
                    }
                    */

                    //cout << "example " << shuffled_examples[example] << ": actual (" << outputs[shuffled_examples[example]][0] << "), node " << current->node << " predicted (" << current->value << "), error: " << node_error << endl;


                    current->error = node_error;

                    if (isnan(error)) {
                        cout << "error became NAN on example " << example << " and iteration " << iteration << " from adding " << (0.5 * (node_error * node_error)) << ", node error: " << node_error << endl;

                        cout << "output layer sum: " << output_layer_sum << endl;
                        cout << "output layer values: " << endl;
                        for (uint32_t i = 0; i < nodes[0][output_layer].size(); i++) {
                            if (nodes[0][output_layer][i] != NULL) {
                                current_output = nodes[0][output_layer][i]->value;

                                cout << "    nodes[0][" << output_layer << "][" << i << "]->value: " << current_output << endl;
                            }
                        }

                        cout << "outputs[example][current->node]: " << outputs[example][current->node] << ", current->value: " << current->value << ", previous value: " << previous_value << endl;
                        cout << "weights into current: " << endl;

                        double current_sum = 0.0;
                        for (uint32_t i = 0; i < current->backward_edges.size(); i++) {
                            EdgeNew *back_edge = current->backward_edges[i];
                            Neuron *current = nodes[back_edge->src_depth][back_edge->src_layer][back_edge->src_node];
                            current_sum += current->value;

                            cout << back_edge << " " << current << ", current sum: " << current_sum << endl;
                        }
                        cout << "sum recalculated: " << current_sum << ", exp(sum)" << exp(current_sum) << endl;

                        cout << "sum isinf? " << isinf(exp(current_sum)) << endl;
                        cout << "sum == numeric_limits::infinity()? " << (exp(current_sum) == numeric_limits<double>::infinity()) << endl;
                        cout << "sum my_isinf? " << my_isinf(exp(current_sum)) << endl;
#ifdef _BOINC_
                        boinc_finish(1);
#endif
                        exit(1);

                        /*
                        ostringstream weights;
                        weights << "starting weights:";
                        for (uint32_t i = 0; i < starting_weights.size(); i++) {
                            weights << " " << starting_weights[i];
                        }
                        cout << weights.str() << endl;
                        */
                    }
                } else {
                    node_error = 0.0;
                    for (uint32_t next = 0; next < current->forward_edges.size(); next++) {
                        dst_depth = current->forward_edges[next]->dst_depth;
                        dst_layer = current->forward_edges[next]->dst_layer;
                        dst_node = current->forward_edges[next]->dst_node;


                        double weight = current->forward_edges[next]->weight;
                        if (isnan(weight) || isinf(weight)) {
                            cout << "NAN/INF on edge: " << current->forward_edges[next]->weight << endl;
                        }

                        double error = nodes[dst_depth][dst_layer][dst_node]->error;
                        if (isnan(error) || isinf(error)) {
                            cout << "NAN/INF on node: " << nodes[dst_depth][dst_layer][dst_node] << endl;
                        }

                        node_error += current->forward_edges[next]->weight * nodes[dst_depth][dst_layer][dst_node]->error;

                        double weight_delta = learning_rate * nodes[dst_depth][dst_layer][dst_node]->error * current->value;
                        if (weight_delta < -0.0001) weight_delta = -0.0001;
                        if (weight_delta > 0.0001) weight_delta = 0.0001;

                        if (weight_delta <= 0 && weight_delta > -0.0000001) weight_delta = -0.0000001;
                        if (weight_delta >= 0 && weight_delta <  0.0000001) weight_delta =  0.0000001;

                        current->forward_edges[next]->weight += weight_delta;

                        if (current->forward_edges[next]->weight > 2.0) current->forward_edges[next]->weight = 2.0;
                        if (current->forward_edges[next]->weight < -2.0) current->forward_edges[next]->weight = -2.0;

                        if (fabs(weight_delta) < min_weight_delta) min_weight_delta = fabs(weight_delta);
                        if (fabs(weight_delta) > max_weight_delta) max_weight_delta = fabs(weight_delta);
                        avg_weight_delta += fabs(weight_delta);
                        avg_weight_delta_count++;
                    }
                    current->error = node_error * derivative_function(current->value);
                }


                current->bias += learning_rate * current->error;

                //cout << current << endl;
            }

            update_recurrent_nodes();
        }

        error /= (-1.0 * inputs.size());

        cout << "[iteration: " << iteration << "] error was: " << setprecision(15) << error
            << ", misclassified examples: " << misclassified_examples
            << ", min_weight_delta: " << min_weight_delta
            << ", avg_weight_delta: " << (avg_weight_delta / avg_weight_delta_count)
            << ", max_weight_delta: " << max_weight_delta << endl;

        /*
        if (error < previous_error) {
            cout << "[iteration: " << iteration << "] breaking because error (" << error << ") < previous error (" << previous_error << ")" << endl;
            return error;

            //learning_rate *= 0.1;
            //cout << "decrementing learning rate to: " << learning_rate << endl;
        }
        */
        previous_error = error;
    }

    return error;
}


bool NeuralNetwork::read_checkpoint(string checkpoint_filename, TaoRandom &generator, vector<uint32_t> &shuffled_examples, uint32_t &iteration) {
    ifstream infile;

#ifdef _BOINC_
    string input_path;
    int retval = boinc_resolve_filename_s(checkpoint_filename.c_str(), input_path);
    if (retval) {
        cerr << "APP: error reading checkpoint (resolving checkpoint file name)" << endl;
        return false;
    }   

    infile.open(input_path.c_str(), ios::in | ios::binary);
#else
    infile.open(checkpoint_filename.c_str(), ios::in | ios::binary);
#endif

    if (!infile.good()) {
        cerr << "could not open checkpoint file for reading: '" << checkpoint_filename << "'" << endl;
        return false;
    } else {
        cerr << "reading checkpoint: '" << checkpoint_filename << "'" << endl;
    }

    reset();


    infile.read( (char*)&iteration, sizeof(uint32_t) );

    uint64_t n_generated;
    infile.read( (char*)&n_generated, sizeof(uint64_t) );
    generator.reset();
    generator.discard(n_generated);

    for (uint32_t i = 0; i < shuffled_examples.size(); i++) {
        infile.read( (char*)&(shuffled_examples[i]), sizeof(uint32_t) );
    }

    uint32_t n_edges, n_recurrent_edges;
    infile.read( (char*)&n_edges, sizeof(uint32_t) );
    infile.read( (char*)&n_recurrent_edges, sizeof(uint32_t) );

    uint32_t src_depth, src_layer, src_node;
    uint32_t dst_depth, dst_layer, dst_node;
    double weight, weight_sum, weight_carry;

    vector<EdgeNew> _edges, _recurrent_edges;

    for (uint32_t i = 0; i < n_edges; i++) {
        infile.read( (char*)&src_depth, sizeof(uint32_t) );
        infile.read( (char*)&src_layer, sizeof(uint32_t) );
        infile.read( (char*)&src_node, sizeof(uint32_t) );
        infile.read( (char*)&dst_depth, sizeof(uint32_t) );
        infile.read( (char*)&dst_layer, sizeof(uint32_t) );
        infile.read( (char*)&dst_node, sizeof(uint32_t) );

        infile.read( (char*)&weight, sizeof(double) );
        //infile.read( (char*)&weight_sum, sizeof(double) );
        //infile.read( (char*)&weight_carry, sizeof(double) );

        _edges.push_back(EdgeNew(src_depth, src_layer, src_node, dst_depth, dst_layer, dst_node, weight));
    }

    if (!infile.good()) {
        cerr << "ERROR reading checkpoint: '" << checkpoint_filename << "' after reading edges" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    for (uint32_t i = 0; i < n_recurrent_edges; i++) {
        infile.read( (char*)&src_depth, sizeof(uint32_t) );
        infile.read( (char*)&src_layer, sizeof(uint32_t) );
        infile.read( (char*)&src_node, sizeof(uint32_t) );
        infile.read( (char*)&dst_depth, sizeof(uint32_t) );
        infile.read( (char*)&dst_layer, sizeof(uint32_t) );
        infile.read( (char*)&dst_node, sizeof(uint32_t) );

        infile.read( (char*)&weight, sizeof(double) );
        //infile.read( (char*)&weight_sum, sizeof(double) );
        //infile.read( (char*)&weight_carry, sizeof(double) );

        _recurrent_edges.push_back(EdgeNew(src_depth, src_layer, src_node, dst_depth, dst_layer, dst_node, weight));
    }
    if (!infile.good()) {
        cerr << "ERROR reading checkpoint: '" << checkpoint_filename << "' after reading recurrent edges" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    set_edges(_edges, _recurrent_edges);

    //needs to be after setting edges because otherwise nodes are not initilalized
    //cout << "reading " << linear_nodes.size() << " nodes." << endl;
    for (uint32_t i = 0; i < linear_nodes.size(); i++) {
        infile.read( (char*)&(linear_nodes[i]->bias), sizeof(double) );
        //cout << "reading bias: " << linear_nodes[i]->bias << endl;
    }

    if (!infile.good()) {
        cerr << "ERROR reading checkpoint: '" << checkpoint_filename << "' after reading bias values" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    infile.close();

    return true;
}

void NeuralNetwork::write_checkpoint(string checkpoint_filename, TaoRandom &generator, const vector<uint32_t> &shuffled_examples, uint32_t iteration) {
    ofstream outfile;

#ifdef _BOINC_
    string output_path;
    int retval = boinc_resolve_filename_s(checkpoint_filename.c_str(), output_path);
    if (retval) {
        cerr << "APP: error writing checkpoint (resolving checkpoint file name)" << endl;
        return;
    }   

    outfile.open(output_path.c_str(), ios::out | ios::binary);
#else
    outfile.open(checkpoint_filename.c_str(), ios::out | ios::binary);
#endif

    if (!outfile.good()) {
        cerr << "could not open checkpoint file for writing: '" << checkpoint_filename << "'" << endl;
        return;
    }


    outfile.write( (char*)&iteration, sizeof(uint32_t) );

    uint64_t n_generated = generator.get_n_generated();
    outfile.write( (char*)&n_generated, sizeof(uint64_t) );

    for (uint32_t i = 0; i < shuffled_examples.size(); i++) {
        outfile.write( (char*)&(shuffled_examples[i]), sizeof(uint32_t) );
    }

    uint32_t n_edges = edges.size();
    uint32_t n_recurrent_edges = recurrent_edges.size();

    outfile.write( (char*)&n_edges, sizeof(uint32_t) );
    outfile.write( (char*)&n_recurrent_edges, sizeof(uint32_t) );

    for (uint32_t i = 0; i < n_edges; i++) {
        EdgeNew *current = edges[i];
        outfile.write( (char*)&(current->src_depth), sizeof(uint32_t) );
        outfile.write( (char*)&(current->src_layer), sizeof(uint32_t) );
        outfile.write( (char*)&(current->src_node), sizeof(uint32_t) );
        outfile.write( (char*)&(current->dst_depth), sizeof(uint32_t) );
        outfile.write( (char*)&(current->dst_layer), sizeof(uint32_t) );
        outfile.write( (char*)&(current->dst_node), sizeof(uint32_t) );

        outfile.write( (char*)&(current->weight), sizeof(double) );
        //outfile.write( (char*)&(current->weight_sum), sizeof(double) );
        //outfile.write( (char*)&(current->weight_carry), sizeof(double) );
    }

    for (uint32_t i = 0; i < n_recurrent_edges; i++) {
        EdgeNew *current = recurrent_edges[i];
        outfile.write( (char*)&(current->src_depth), sizeof(uint32_t) );
        outfile.write( (char*)&(current->src_layer), sizeof(uint32_t) );
        outfile.write( (char*)&(current->src_node), sizeof(uint32_t) );
        outfile.write( (char*)&(current->dst_depth), sizeof(uint32_t) );
        outfile.write( (char*)&(current->dst_layer), sizeof(uint32_t) );
        outfile.write( (char*)&(current->dst_node), sizeof(uint32_t) );

        outfile.write( (char*)&(current->weight), sizeof(double) );
        //outfile.write( (char*)&(current->weight_sum), sizeof(double) );
        //outfile.write( (char*)&(current->weight_carry), sizeof(double) );
    }

    //cout << "writing " << linear_nodes.size() << " nodes." << endl;
    for (uint32_t i = 0; i < linear_nodes.size(); i++) {
        //cout << "writing bias: " << linear_nodes[i]->bias << endl;
        outfile.write( (char*)&(linear_nodes[i]->bias), sizeof(double) );
    }

    outfile.close();
}

ostream& operator<< (ostream& out, const NeuralNetwork &neural_network) {
    return out;
}


ostream& operator<< (ostream& out, const NeuralNetwork *neural_network) {
    return out;
}

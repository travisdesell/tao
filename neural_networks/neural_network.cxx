#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "./neural_networks/edge_new.hxx"
#include "./neural_networks/neural_network.hxx"

Neuron::Neuron(uint32_t _depth, uint32_t _layer, uint32_t _node) : depth(_depth), layer(_layer), node(_node), identifier(""), bias(0.0), value(0.0) {
}

void Neuron::connect_forward(EdgeNew edge) {
    forward_edges.push_back(edge);
}

void Neuron::connect_backward(EdgeNew edge) {
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
        << "\"value\" : " << value
        << "}";

    return oss.str();
}

ostream& operator<< (ostream& out, const Neuron &neuron) {
    out << neuron.json() << endl;
    return out;
}

ostream& operator<< (ostream& out, const Neuron *neuron) {
    out << neuron->json() << endl;
    return out;
}


NeuralNetwork::NeuralNetwork(string json_filename) {
}

NeuralNetwork::NeuralNetwork(uint32_t _recurrent_depth, uint32_t _n_input_nodes, uint32_t _n_hidden_layers, uint32_t _n_hidden_nodes, uint32_t _n_output_nodes) : recurrent_depth(_recurrent_depth), n_input_nodes(_n_input_nodes), n_hidden_layers(_n_hidden_layers), n_hidden_nodes(_n_hidden_nodes), n_output_nodes(_n_output_nodes) {

    //initialize all the possible nodes to null
    for (uint32_t depth = 0; depth < recurrent_depth; depth++) {
        nodes.push_back( vector< vector<Neuron*> >() );

        uint32_t layer = 0;
        nodes[depth].push_back( vector<Neuron*>() );

        for (uint32_t node = 0; node < n_input_nodes; node++) {
            nodes.at(depth).at(layer).push_back( NULL );
        }

        layer = 1;
        for (; layer < n_hidden_layers + 1; layer++) {
            nodes.at(depth).push_back( vector<Neuron*>() );
            for (uint32_t node = 0; node < n_hidden_nodes; node++) {
                nodes.at(depth).at(layer).push_back( NULL );
            }
        }

        //no recurrent layer on the output layer
        if (depth == 0) {
            nodes.at(depth).push_back( vector<Neuron*>() );
            for (uint32_t node = 0; node < n_output_nodes; node++) {
                nodes.at(depth).at(layer).push_back( NULL );
            }
        }
    }

    for (uint32_t i = 0; i < nodes.size(); i++) {
        cout << "depth[" << i << "/" << nodes.size() << "]" << endl;
        for (uint32_t j = 0; j < nodes[i].size(); j++) {
            cout << "    layer[" << j << "/" << nodes[i].size() << "] size: " << nodes[i][j].size() << endl;
        }
    }


}

void NeuralNetwork::set_training_data(uint32_t n_examples, uint32_t input_size, const double **_inputs, uint32_t output_size, double **_outputs) {
    inputs = vector< vector<double> >(n_examples, vector<double>(input_size, 0.0));
    outputs = vector< vector<double> >(n_examples, vector<double>(output_size, 0.0));

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
    edges = _edges;
    recurrent_edges = _recurrent_edges;

    uint32_t src_depth, src_layer, src_node;
    uint32_t dst_depth, dst_layer, dst_node;

    Neuron *src_neuron, *dst_neuron;

    cout << "setting edges: " << endl;
    for (uint32_t edge = 0; edge < edges.size(); edge++) {
        cout << "setting edge: " << edges.at(edge) << endl;

        src_depth = edges.at(edge).src_depth;
        src_layer = edges.at(edge).src_layer;
        src_node = edges.at(edge).src_node;

        cout << "setting source neuron" << endl;
        src_neuron = NULL;
        if (nodes.at(src_depth).at(src_layer).at(src_node) == NULL) {
            src_neuron = new Neuron(src_depth, src_layer, src_node);
            nodes.at(src_depth).at(src_layer).at(src_node) = src_neuron;
        } else {
            src_neuron = nodes.at(src_depth).at(src_layer).at(src_node);
        }

        dst_depth = edges.at(edge).dst_depth;
        dst_layer = edges.at(edge).dst_layer;
        dst_node = edges.at(edge).dst_node;

        cout << "setting dst neuron" << endl;
        dst_neuron = NULL;
        if (nodes.at(dst_depth).at(dst_layer).at(dst_node) == NULL) {
            dst_neuron = new Neuron(dst_depth, dst_layer, dst_node);
            nodes.at(dst_depth).at(dst_layer).at(dst_node) = dst_neuron;
        } else {
            dst_neuron = nodes.at(dst_depth).at(dst_layer).at(dst_node);
        }

        cout << "connecting forward" << endl;
        src_neuron->connect_forward(edges.at(edge));

        cout << "connecting backward" << endl;
        dst_neuron->connect_backward(edges.at(edge));
    }
    cout << "set edges." << endl;

    cout << "setting recurrent edges: " << endl;
    for (uint32_t edge = 0; edge < recurrent_edges.size(); edge++) {
        cout << "setting edge: " << recurrent_edges.at(edge) << endl;

        src_depth = recurrent_edges.at(edge).src_depth;
        src_layer = recurrent_edges.at(edge).src_layer;
        src_node = recurrent_edges.at(edge).src_node;

        cout << "setting source neuron" << endl;
        src_neuron = NULL;
        if (nodes.at(src_depth).at(src_layer).at(src_node) == NULL) {
            src_neuron = new Neuron(src_depth, src_layer, src_node);
            nodes.at(src_depth).at(src_layer).at(src_node) = src_neuron;
        } else {
            src_neuron = nodes.at(src_depth).at(src_layer).at(src_node);
        }

        dst_depth = recurrent_edges.at(edge).dst_depth;
        dst_layer = recurrent_edges.at(edge).dst_layer;
        dst_node = recurrent_edges.at(edge).dst_node;

        cout << "setting dst neuron" << endl;
        dst_neuron = NULL;
        if (nodes.at(dst_depth).at(dst_layer).at(dst_node) == NULL) {
            dst_neuron = new Neuron(dst_depth, dst_layer, dst_node);
            nodes.at(dst_depth).at(dst_layer).at(dst_node) = dst_neuron;
        } else {
            dst_neuron = nodes.at(dst_depth).at(dst_layer).at(dst_node);
        }

        cout << "connecting forward" << endl;
        src_neuron->connect_forward(recurrent_edges.at(edge));

        cout << "connecting backward" << endl;
        dst_neuron->connect_backward(recurrent_edges.at(edge));
    }
    cout << "set recurrent edges." << endl;
}

void NeuralNetwork::set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges, const vector<string> &input_labels, const vector<string> &output_labels) {
    cout << "calling set_edges." << endl;
    set_edges(_edges, _recurrent_edges);
    cout << "called set_edges." << endl;

    if (nodes.at(0).at(0).size() != input_labels.size()) {
        cerr << "ERROR on '" << __FILE__ << ":" << __LINE__ << "':" << endl;
        cerr << "\tinput label size (" << input_labels.size() << ") does not match nodes[0][0].size (" << nodes.at(0).at(0).size() << ")" << endl;
        exit(1);
    }

    if (nodes.at(0).at(nodes.at(0).size() - 1).size() != output_labels.size()) {
        cerr << "ERROR on '" << __FILE__ << ":" << __LINE__ << "':" << endl;
        cerr << "\toutput label size (" << output_labels.size() << ") does not match nodes[0][nodes.size() - 1].size (" << nodes.at(0).at(nodes.at(0).size() - 1).size() << ")" << endl;
        exit(1);
    }

    for (uint32_t i = 0; i < input_labels.size(); i++) {
        if (nodes.at(0).at(0).at(i) != NULL) {
            nodes.at(0).at(0).at(i)->identifier = input_labels.at(i);
        }
    }

    for (uint32_t i = 0; i < output_labels.size(); i++) {
        if (nodes.at(0).at(nodes.at(0).size() - 1).at(i) != NULL) {
            nodes.at(0).at(nodes.at(0).size() - 1).at(i)->identifier = output_labels.at(i);
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
    oss << "    \"nodes\" : {" << endl;

    bool printed_previously = false;
    for (uint32_t i = 0; i < nodes.size(); i++) {
        for (uint32_t j = 0; j < nodes.at(i).size(); j++) {
            for (uint32_t k = 0; k < nodes.at(i).at(j).size(); k++) {

                if (nodes.at(i).at(j).at(k) != NULL) {
                    if (printed_previously) oss << "," << endl << "        " << nodes.at(i).at(j).at(k)->json();
                    else oss << "        " << nodes.at(i).at(j).at(k)->json();
                    printed_previously = true;
                }
            }
        }
    }
    oss << endl;
    oss << "    }," << endl;

    oss << "    \"edges\" : {" << endl;
    for (uint32_t i = 0; i < edges.size(); i++) {
        oss << "        " << edges.at(i).json();
        if (i != edges.size() - 1) oss << ", " << endl;
        else oss << endl;
    }
    oss << "    }," << endl;

    oss << "    \"recurrent_edges\" : {" << endl;
    for (uint32_t i = 0; i < recurrent_edges.size(); i++) {
        oss << "        " << recurrent_edges.at(i).json();
        if (i != recurrent_edges.size() - 1) oss << ", " << endl;
        else oss << endl;
    }
    oss << "    }" << endl;
    oss << "}" << endl;

    return oss.str();
}

void NeuralNetwork::write_to_file(string json_filename) {
}

void NeuralNetwork::get_gradient(vector<double> &gradient) {
}

void NeuralNetwork::get_gradient_stochastic(vector<double> &gradient) {
}

ostream& operator<< (ostream& out, const NeuralNetwork &neural_network) {
    return out;
}


ostream& operator<< (ostream& out, const NeuralNetwork *neural_network) {
    return out;
}

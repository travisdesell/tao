#include <cmath>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <fstream>
using std::ofstream;

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
        << "\"value\" : " << value
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

    /*
    for (uint32_t i = 0; i < nodes.size(); i++) {
        cout << "depth[" << i << "/" << nodes.size() << "]" << endl;
        for (uint32_t j = 0; j < nodes[i].size(); j++) {
            cout << "    layer[" << j << "/" << nodes[i].size() << "] size: " << nodes[i][j].size() << endl;
        }
    }
    */


}

void NeuralNetwork::set_training_data(uint32_t n_examples, uint32_t input_size, double **_inputs, uint32_t output_size, double **_outputs) {
    inputs = vector< vector<double> >(n_examples, vector<double>(input_size, 0.0));
    outputs = vector< vector<double> >(n_examples, vector<double>(output_size, 0.0));

    /*
    cout << "setting training data, "
         << " input size[" << inputs.size() << "][" << inputs.at(0).size() << "], "
         << " output size[" << outputs.size() << "][" << outputs.at(0).size() << "]" << endl;
    */

    for (uint32_t i = 0; i < n_examples; i++) {
        for (uint32_t j = 0; j < input_size; j++) {
            inputs.at(i).at(j) = _inputs[i][j];
        }

        for (uint32_t j = 0; j < output_size; j++) {
            outputs.at(i).at(j) = _outputs[i][j];
        }
    }
}

void NeuralNetwork::set_training_data(const vector< vector<double> > &_inputs, const vector< vector<double> > &_outputs) {
    inputs = _inputs;
    outputs = _outputs;
}

void NeuralNetwork::set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges) {
    for (uint32_t i = 0; i < _edges.size(); i++) {
        edges.push_back(new EdgeNew(_edges.at(i).src_depth, _edges.at(i).src_layer, _edges.at(i).src_node, _edges.at(i).dst_depth, _edges.at(i).dst_layer, _edges.at(i).dst_node, _edges.at(i).weight));
    }

    for (uint32_t i = 0; i < _recurrent_edges.size(); i++) {
        recurrent_edges.push_back(new EdgeNew(_recurrent_edges.at(i).src_depth, _recurrent_edges.at(i).src_layer, _recurrent_edges.at(i).src_node, _recurrent_edges.at(i).dst_depth, _recurrent_edges.at(i).dst_layer, _recurrent_edges.at(i).dst_node, _recurrent_edges.at(i).weight));
    }

    uint32_t src_depth, src_layer, src_node;
    uint32_t dst_depth, dst_layer, dst_node;

    Neuron *src_neuron, *dst_neuron;

    //cout << "setting edges: " << endl;
    for (uint32_t edge = 0; edge < edges.size(); edge++) {
        //cout << "setting edge: " << edges.at(edge) << endl;

        src_depth = edges.at(edge)->src_depth;
        src_layer = edges.at(edge)->src_layer;
        src_node = edges.at(edge)->src_node;

        //cout << "setting source neuron" << endl;
        src_neuron = NULL;
        if (nodes.at(src_depth).at(src_layer).at(src_node) == NULL) {
            src_neuron = new Neuron(src_depth, src_layer, src_node);
            nodes.at(src_depth).at(src_layer).at(src_node) = src_neuron;
        } else {
            src_neuron = nodes.at(src_depth).at(src_layer).at(src_node);
        }

        dst_depth = edges.at(edge)->dst_depth;
        dst_layer = edges.at(edge)->dst_layer;
        dst_node = edges.at(edge)->dst_node;

        //cout << "setting dst neuron" << endl;
        dst_neuron = NULL;
        if (nodes.at(dst_depth).at(dst_layer).at(dst_node) == NULL) {
            dst_neuron = new Neuron(dst_depth, dst_layer, dst_node);
            nodes.at(dst_depth).at(dst_layer).at(dst_node) = dst_neuron;
        } else {
            dst_neuron = nodes.at(dst_depth).at(dst_layer).at(dst_node);
        }

        //cout << "connecting forward" << endl;
        src_neuron->connect_forward(edges.at(edge));

        //cout << "connecting backward" << endl;
        dst_neuron->connect_backward(edges.at(edge));
    }
    //cout << "set edges." << endl;

    //cout << "setting recurrent edges: " << endl;
    for (uint32_t edge = 0; edge < recurrent_edges.size(); edge++) {
        //cout << "setting edge: " << recurrent_edges.at(edge) << endl;

        src_depth = recurrent_edges.at(edge)->src_depth;
        src_layer = recurrent_edges.at(edge)->src_layer;
        src_node = recurrent_edges.at(edge)->src_node;

        //cout << "setting source neuron" << endl;
        src_neuron = NULL;
        if (nodes.at(src_depth).at(src_layer).at(src_node) == NULL) {
            src_neuron = new Neuron(src_depth, src_layer, src_node);
            nodes.at(src_depth).at(src_layer).at(src_node) = src_neuron;
        } else {
            src_neuron = nodes.at(src_depth).at(src_layer).at(src_node);
        }

        dst_depth = recurrent_edges.at(edge)->dst_depth;
        dst_layer = recurrent_edges.at(edge)->dst_layer;
        dst_node = recurrent_edges.at(edge)->dst_node;

        //cout << "setting dst neuron" << endl;
        dst_neuron = NULL;
        if (nodes.at(dst_depth).at(dst_layer).at(dst_node) == NULL) {
            dst_neuron = new Neuron(dst_depth, dst_layer, dst_node);
            nodes.at(dst_depth).at(dst_layer).at(dst_node) = dst_neuron;
        } else {
            dst_neuron = nodes.at(dst_depth).at(dst_layer).at(dst_node);
        }

        //cout << "connecting forward" << endl;
        src_neuron->connect_forward(recurrent_edges.at(edge));

        //cout << "connecting backward" << endl;
        dst_neuron->connect_backward(recurrent_edges.at(edge));
    }
    //cout << "set recurrent edges." << endl;

    active_input_nodes = 0;
    for (uint32_t depth = 0; depth < nodes.size(); depth++) {
        for (uint32_t layer = 0; layer < nodes.at(depth).size(); layer++) {
            for (uint32_t node = 0; node < nodes.at(depth).at(layer).size(); node++) {
                if (nodes.at(depth).at(layer).at(node) != NULL) {
                    if (layer == 0) active_input_nodes++;
                    linear_nodes.push_back(nodes.at(depth).at(layer).at(node));
                }
            }
        }
    }

    output_layer = nodes.at(0).size() - 1;
}

void NeuralNetwork::set_edges(const vector<EdgeNew> &_edges, const vector<EdgeNew> &_recurrent_edges, const vector<string> &input_labels, const vector<string> &output_labels) {
    set_edges(_edges, _recurrent_edges);

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
    oss << "    \"nodes\" : [" << endl;

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
    oss << "    ]," << endl;

    oss << "    \"edges\" : [" << endl;
    for (uint32_t i = 0; i < edges.size(); i++) {
        oss << "        " << edges.at(i)->json();
        if (i != edges.size() - 1) oss << ", " << endl;
        else oss << endl;
    }
    oss << "    ]," << endl;

    oss << "    \"recurrent_edges\" : [" << endl;
    for (uint32_t i = 0; i < recurrent_edges.size(); i++) {
        oss << "        " << recurrent_edges.at(i)->json();
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
        exit(1);
    }

    outfile.close();
}

uint32_t NeuralNetwork::get_parameter_size() {
    //input nodes have no bias
    return linear_nodes.size() + edges.size() - active_input_nodes;
}

double NeuralNetwork::evaluate(const vector<double> &weights) {
    if (weights.size() != get_parameter_size()) {
        cerr << "ERROR on '" << __FILE__ << ":" << __LINE__ << "':" << endl;
        cerr << "\tnumber of weights to evaluate neural network with (" << weights.size() << ") != number of nodes (" << linear_nodes.size() << ") + number of edges (" << edges.size() << ")" << endl;
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
        linear_nodes.at(node)->value = 0;

        //input nodes have no bias
        if (linear_nodes.at(node)->layer != 0) {
            linear_nodes.at(node)->bias = weights[current];
            current++;
        }

    }

    for (uint32_t edge = 0; edge < edges.size(); edge++) {
        edges.at(edge)->weight = weights[current];
        current++;
    }
    if (current != weights.size()) {
        cerr << "ERROR: current (" << current << ") != weights.size() (" << weights.size() << ")" << endl;
        exit(1);
    }

    //recurrent edges have no weight

    double error = 0.0;
    for (uint32_t example = 0; example < inputs.size(); example++) {

        //set initial bias and input values
        current = 0;
        for (uint32_t node = 0; node < linear_nodes.size(); node++) {
            if (linear_nodes.at(node)->depth == 0) {
                if (linear_nodes.at(node)->layer == 0) {
                    linear_nodes.at(node)->value = inputs.at(example).at(linear_nodes.at(node)->node);
                } else {
                    //reset non-recurrent node values equal to bias
                    linear_nodes.at(node)->value = linear_nodes.at(node)->bias;
                    current++;
                }
            } else {
                //add bias to recurrent node values
                linear_nodes.at(node)->value += linear_nodes.at(node)->bias;
            }
//            cout << linear_nodes.at(node) << endl;
        }

//        cout << "output node value: " << nodes.at(0).at(output_layer).at(0)->value << endl;

        //propogate values forward
        for (uint32_t i = 0; i < edges.size(); i++) {
            //cout << "node value " << nodes.at(edges.at(i)->dst_depth).at(edges.at(i)->dst_layer).at(edges.at(i)->dst_node)->value
            //     << " += " << edges.at(i)->weight
            //     << " * " << nodes.at(edges.at(i)->src_depth).at(edges.at(i)->src_layer).at(edges.at(i)->src_node)->value
            //     << endl;

            nodes.at(edges.at(i)->dst_depth).at(edges.at(i)->dst_layer).at(edges.at(i)->dst_node)->value += edges.at(i)->weight * nodes.at(edges.at(i)->src_depth).at(edges.at(i)->src_layer).at(edges.at(i)->src_node)->value;

        }

        //calculate the error
        for (uint32_t i = 0; i < nodes.at(0).at(output_layer).size(); i++) {
            //double example_error = (outputs[example][i] - nodes.at(0).at(output_layer).at(i)->value) * (outputs[example][i] - nodes.at(0).at(output_layer).at(i)->value);
            double example_error = fabs(outputs[example][i] - nodes.at(0).at(output_layer).at(i)->value);

//            cout << "example " << example << ": actual (" << outputs[example][i] << "), predicted (" << nodes.at(0).at(output_layer).at(i)->value << "), error: " << example_error << endl;

            error += example_error;
        }

        //copy values to recurrent neurons, starting at the deepest layer any working up
        for (int32_t i = recurrent_edges.size() - 1; i >= 0; i--) {
            nodes.at(recurrent_edges.at(i)->dst_depth).at(recurrent_edges.at(i)->dst_layer).at(recurrent_edges.at(i)->dst_node)->value = nodes.at(recurrent_edges.at(i)->src_depth).at(recurrent_edges.at(i)->src_layer).at(recurrent_edges.at(i)->src_node)->value;

            if (nodes.at(recurrent_edges.at(i)->dst_depth).at(recurrent_edges.at(i)->dst_layer).at(recurrent_edges.at(i)->dst_node)->value > 5000.00) {
                nodes.at(recurrent_edges.at(i)->dst_depth).at(recurrent_edges.at(i)->dst_layer).at(recurrent_edges.at(i)->dst_node)->value = 5000.00;
                //cout << "recurrent node too big!" << endl;
            }

            if (nodes.at(recurrent_edges.at(i)->dst_depth).at(recurrent_edges.at(i)->dst_layer).at(recurrent_edges.at(i)->dst_node)->value < -5000.00) {
                nodes.at(recurrent_edges.at(i)->dst_depth).at(recurrent_edges.at(i)->dst_layer).at(recurrent_edges.at(i)->dst_node)->value = -5000.00;
                //cout << "recurrent node too small!" << endl;
            }

//            cout << "set recurrent node: " << nodes.at(recurrent_edges.at(i)->dst_depth).at(recurrent_edges.at(i)->dst_layer).at(recurrent_edges.at(i)->dst_node) << endl;
        }
    }
    error /= (-1.0 * inputs.size());
//    cout << "error was: " << error << endl;

    return error;
}

void NeuralNetwork::get_gradient(vector<double> &gradient) {

    //do a forward pass to calculate the error at the output layer

    //where f' is the derivative of the activation function:
    //the error of the output nodes is: (output - predicted output)

    //do backpropagation, the error of each node is is equal to f'(node) * Sum(error of posterior nodes * weight to posterior node) -- 

    //gradient for each weight is equal to ?
}

void NeuralNetwork::get_gradient_stochastic(vector<double> &gradient) {
}

ostream& operator<< (ostream& out, const NeuralNetwork &neural_network) {
    return out;
}


ostream& operator<< (ostream& out, const NeuralNetwork *neural_network) {
    return out;
}

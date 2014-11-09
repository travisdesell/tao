import java.io.BufferedReader;
import java.io.FileReader;

import java.util.ArrayList;


int height = 800;
int width = 1000;

size(width, height);


ArrayList<ArrayList<Double>> pheromones = new ArrayList<ArrayList<Double>>();

int neuron_width = 30;
int neuron_height = 30;

int neuron_distance = 70;

int current_x = 0;
int current_y = 20;

int max_pheromone = 10;

try {
    BufferedReader br = new BufferedReader(new FileReader("/Users/tdesell/Code/neural_network_visualizer/single_nn_8.txt"));
    //BufferedReader br = new BufferedReader(new FileReader("/Users/tdesell/Code/neural_network_visualizer/aco_output_nhl_3_npl_8_ants_16"));

    try {
        StringBuilder sb = new StringBuilder();
        String line = "";

        int src_x = 0, src_y = 0, dst_x = 0, dst_y = 0;

        while (line != null) {
            line = br.readLine();
            System.out.println("line: '" + line + "'");

            if (line.equals("")) {
                continue;
            } else if (line.charAt(0) == 'f') {
                continue;
            } else if (line.charAt(0) == 'i') {
                continue;
            } else if (line.charAt(0) == '[') {
                continue;
            } else if (line.equals("#pheromones")) {
                continue;
            }

            if (line.charAt(0) == '#') {
                String[] splits = line.split("[#\\[\\]:]");
                //for (String s : splits) System.out.println(s);
                src_x = Integer.parseInt(splits[2]);
                src_y = Integer.parseInt(splits[4]);

                System.out.println("drawing neuron: " + src_x  + ", " + src_y);

                current_x = (src_x * neuron_distance) + neuron_distance;
                current_y = (src_y * neuron_distance) + neuron_distance;

                stroke(0);
                ellipse(current_y, current_x, neuron_width, neuron_height);

            } else {
                String[] splits = line.split("[#\\[\\]:]");
                //for (String s : splits) System.out.println(s);
                dst_x = Integer.parseInt(splits[1]);
                dst_y = Integer.parseInt(splits[3]);
                double pheromone = Double.parseDouble(splits[5]);

                System.out.println("\tdrawing link from: [" + src_x + "][" + src_y + "] to [" + dst_x + "][" + dst_y + "]");

                int start_x = (src_x * neuron_distance) + neuron_distance;
                int start_y = (src_y * neuron_distance) + neuron_distance;
                int end_x = (dst_x * neuron_distance) + neuron_distance;
                int end_y = (dst_y * neuron_distance) + neuron_distance;

                if (pheromone > 2) {
                    stroke((int)(255 - ((pheromone / max_pheromone) * 255)));
                    line(start_y, start_x, end_y, end_x);
                }
            }

            System.out.println(line);
        }

    } finally {
        br.close();
    }
} catch (Exception e) {
    println("EXCEPTION: " + e);
    e.printStackTrace();
}

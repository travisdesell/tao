import java.io.BufferedReader;
import java.io.FileReader;

import java.util.ArrayList;

int output_layer;

int height =1200;
int width = 1000;

size(width, height);
background(255);


ArrayList<ArrayList<Double>> pheromones = new ArrayList<ArrayList<Double>>();

int neuron_width = 30;
int neuron_height = 30;

int neuron_distance = 60;

int current_x = 0;
int current_y = 20;

int max_pheromone = 10;

class Edge {
  int src_layer, dst_layer, src_node, dst_node;
  double weight;

  public Edge(int src_layer, int dst_layer, int src_node, int dst_node, double weight) {
    this.src_layer = src_layer;
    this.dst_layer = dst_layer;
    this.src_node = src_node;
    this.dst_node = dst_node;
    this.weight = weight;
  }
}

try {
    //BufferedReader br = new BufferedReader(new FileReader("/Users/deselt/Code/flight_analysis/test_networks/fully_connected_elman_h1_n4.txt"));
    BufferedReader br = new BufferedReader(new FileReader("/Users/deselt/Code/flight_analysis/aco_output/40_2"));

    try {
        String line = br.readLine();
        line = br.readLine();

        int n_hidden_layers = Integer.parseInt(line);

        line = br.readLine();
        line = br.readLine();
        line = br.readLine();
        int nodes_per_layer = Integer.parseInt(line);
        nodes_per_layer += 1; // for bias nodes

        System.out.println("n_hidden_layers: " + n_hidden_layers);
        System.out.println("nodes_per_layer: " + nodes_per_layer);

        line = br.readLine();
        line = br.readLine();


        int[][] possible_nodes = new int[(n_hidden_layers * 2) + 2][];
        for (int i = 0; i < possible_nodes.length; i++) {
          possible_nodes[i] = new int[nodes_per_layer];
          for (int j = 0; j < possible_nodes[i].length; j++) {
            possible_nodes[i][j] = 0;
          }

//          if ((i % 2) != 1) {  //no bias node on recurrent layers
//            possible_nodes[i][ possible_nodes[i].length - 1] = 1;
//          }
        }

        output_layer = possible_nodes.length - 1;

        ArrayList<Edge> edges = new ArrayList<Edge>();

        line = br.readLine();
        while (!line.equals("")) {
//            System.out.println("line: '" + line + "'");

            String[] splits = line.split(" ");
            //for (String s : splits) System.out.println(s);

            int src_layer = Integer.parseInt(splits[0]);
            int dst_layer = Integer.parseInt(splits[1]);
            int src_node = Integer.parseInt(splits[2]);
            int dst_node = Integer.parseInt(splits[3]);
            double weight = 0;
            if (splits.length >= 5) weight = Double.parseDouble(splits[4]);

            possible_nodes[src_layer][src_node] = 1;
            possible_nodes[dst_layer][dst_node] = 1;

            edges.add( new Edge(src_layer, dst_layer, src_node, dst_node, weight) );

            System.out.println("read edge: " + src_layer + " " + dst_layer + " " + src_node + " " + dst_node + " " + weight);

            line = br.readLine();
        }

        line = br.readLine();
        line = br.readLine();
        while (line != null && !line.equals("")) {
//            System.out.println("line: '" + line + "'");

            String[] splits = line.split(" ");
            //for (String s : splits) System.out.println(s);

            int src_layer = Integer.parseInt(splits[0]);
            int dst_layer = Integer.parseInt(splits[1]);
            int src_node = Integer.parseInt(splits[2]);
            int dst_node = Integer.parseInt(splits[3]);
            double weight = 0;
            if (splits.length >= 5) weight = Double.parseDouble(splits[4]);

            possible_nodes[src_layer][src_node] = 1;
            possible_nodes[dst_layer][dst_node] = 1;

            edges.add( new Edge(src_layer, dst_layer, src_node, dst_node, weight) );

            System.out.println("read recurrent edge: " + src_layer + " " + dst_layer + " " + src_node + " " + dst_node + " " + weight);

            line = br.readLine();
        }

        System.out.println("Possible Nodes:");

        for (int i = 0; i < possible_nodes.length; i++) {
          for (int j = 0; j < possible_nodes[i].length; j++) {
            System.out.print(" " + possible_nodes[i][j]);
          }
          System.out.println();
        }

        for (int i = 0; i < possible_nodes.length; i++) {
          for (int j = 0; j < possible_nodes[i].length; j++) {
            if (possible_nodes[i][j] == 0) continue;

            stroke(0);

            int src_x = i;
            int src_y = j;

            System.out.println("drawing neuron: " + src_x  + ", " + src_y);

            current_x = (src_x * neuron_distance) + neuron_distance;
            current_y = (src_y * neuron_distance) + neuron_distance;

            if (src_x % 2 == 1 && src_x != output_layer) {
              current_x += neuron_distance;
              current_y += (9 * neuron_distance);
            }   

            if (src_y == possible_nodes[src_x].length - 1) {
              fill(200, 50, 50);
            } else if (src_x == 0) {
              fill(50, 50, 200);
            } else if (src_x == output_layer) {
              fill(50, 200, 50);
            } else if (src_x % 2 == 1) {
              fill(200, 200, 128);
            } else {
              fill(128, 200, 200);
            }   

            ellipse(current_x, current_y, neuron_width, neuron_height);
          }
        }

        for (int i = 0; i < edges.size(); i++) {
          int src_x = edges.get(i).src_layer;
          int src_y = edges.get(i).src_node;
          int dst_x = edges.get(i).dst_layer;
          int dst_y = edges.get(i).dst_node;
          double weight = edges.get(i).weight;

          System.out.println("\tdrawing link from: [" + src_x + "][" + src_y + "] to [" + dst_x + "][" + dst_y + "] with weight: " + weight);

          int start_x = (src_x * neuron_distance) + neuron_distance;
          int start_y = (src_y * neuron_distance) + neuron_distance;
          int end_x = (dst_x * neuron_distance) + neuron_distance;
          int end_y = (dst_y * neuron_distance) + neuron_distance;

          if (src_x % 2 == 1 && src_x != output_layer) {
            start_x += neuron_distance;
            start_y += (9 * neuron_distance);
          }

          if (dst_x % 2 == 1 && dst_x != output_layer) {
            end_x += neuron_distance;
            end_y += (9 * neuron_distance);
          }

//          if (weight == 0) {
//            stroke(50);
//          } else {
          if (weight < 0.00001) {
            continue;
          }
          stroke(max(0, 255 - Math.abs((int)(weight * 200.0)) ));
//          }

          if (src_x % 2 == 1) {
            int mid_x1 = start_x + (3 * neuron_distance);
            int mid_x2 = start_x + (3 * neuron_distance);
            //int mid_y1 = Math.abs(start_y - end_y) / 2;
            //int mid_y2 = Math.abs(start_y - end_y) / 2;
            int mid_y1 = end_y;
            int mid_y2 = start_y;

            //curve(start_x, start_y, mid_x1, mid_y1, mid_x2, mid_y2, end_x, end_y);
            noFill();
            curve(mid_x1, mid_y1, start_x, start_y, end_x, end_y, mid_x2, mid_y2);

          } else if (dst_x % 2 == 1 && dst_x != output_layer) {
            int mid_x1 = start_x - (3 * neuron_distance);
            int mid_x2 = start_x - (3 * neuron_distance);
            //int mid_y1 = Math.abs(start_y - end_y) / 2;
            //int mid_y2 = Math.abs(start_y - end_y) / 2;
            int mid_y1 = end_y;
            int mid_y2 = start_y;

            noFill();
            curve(mid_x1, mid_y1, start_x, start_y, end_x, end_y, mid_x2, mid_y2);

          } else {
            line(start_x, start_y, end_x, end_y);
          }
        }

    } finally {
        br.close();
    }
} catch (Exception e) {
    println("EXCEPTION: " + e);
    e.printStackTrace();
}

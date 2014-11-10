import java.io.BufferedReader;
import java.io.FileReader;

import java.util.ArrayList;

int output_layer = 7;

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

try {
    BufferedReader br = new BufferedReader(new FileReader("/Users/deselt/Code/tao/visualization/single_nn_8.txt"));
    //BufferedReader br = new BufferedReader(new FileReader("/Users/deselt/Code/tao/visualization/aco_output_nhl_3_npl_8_ants_16"));

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

                if (src_x % 2 == 1 && src_x != output_layer) {
                  current_x += neuron_distance;
                  current_y += (9 * neuron_distance);
                }

                stroke(0);

                if (src_x == 0) {
                  fill(50, 50, 200);
                } else if (src_x == output_layer) {
                  fill(50, 200, 50);
                } else if (src_x % 2 == 1) {
                  fill(200, 200, 128);
                } else {
                  fill(128, 200, 200);
                }

                ellipse(current_x, current_y, neuron_width, neuron_height);

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

                if (src_x % 2 == 1 && src_x != output_layer) {
                  start_x += neuron_distance;
                  start_y += (9 * neuron_distance);
                }

                if (dst_x % 2 == 1 && dst_x != output_layer) {
                  end_x += neuron_distance;
                  end_y += (9 * neuron_distance);
                }

                if (pheromone > 2) {
                    stroke((int)(255 - ((pheromone / max_pheromone) * 255)));

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

                    /*
                    pushMatrix();
                      translate(end_x, end_y);
                      float a = atan2(start_x-end_x, start_y-end_y);
                      rotate(a);
                      line(0, 0, -4, -4);
                      line(0, 0, 4, -4);
                    popMatrix();
                    */

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

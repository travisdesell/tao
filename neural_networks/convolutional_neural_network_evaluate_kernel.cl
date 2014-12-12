/*
 *  Need to load images into constant memory (these won't change)
 *
 *  Need to load weights into constant memory (won't change per run)
 *
 *  Nodes need to be local memory (if there is enougH)
 */

__kernel void add_numbers(__global char *image, __constant int *nn_params, __constant float *weights, __local float* nodes, __local float* fc_layer, __global float* output_classifications) {
//    int grp_id_x = get_group_id(0);
//    int lcl_id_x = get_local_id(0);
//    int lcl_sz_x = get_local_size(0);

//    int image_pos = (lcl_sz_x * grp_id_x) + lcl_id_x;
//    printf("image_pos: %d\n", image_pos);

    int image_pos = get_group_id(0);

    int image_x = nn_params[0];
    int image_y = nn_params[1];

    int n_layers = nn_params[2];
    int fc_size = nn_params[3];
    int n_classes = nn_params[4];

//    if (image_pos >= nn_params[4 + (n_layers * 2) + n_classes]) return;

    //these need to be input parameters somehow
    /*
    int image_x = 16;
    int image_y = 16;

    int conv_size = 4;
    int pool_size = 2;
    int fc_size = 9;
    int n_classes = 3;
    int classification = 0;
    */

    int current_image_pos = image_pos * (image_x * image_y * 3);
    int pos = 0;
    float tmp;
    for (int i = 0; i < image_x; i++) {
        for (int j = 0; j < image_y; j++) {
            tmp = 0.0f;

            tmp  = weights[0] * (image[current_image_pos] / 256.0f);
            tmp += weights[1] * (image[current_image_pos + 1] / 256.0f);
            tmp += weights[2] * (image[current_image_pos + 2] / 256.0f);
            tmp += weights[3];
            tmp = 1.0f / (1.0f + exp(-tmp));
            nodes[pos] = tmp;
            pos++;
            current_image_pos += 3;

        }
    }
    int current_weight = 4;


    for (int layer = 0; layer < n_layers; layer++) {
        int conv_size = nn_params[5 + (layer * 2)    ];
        int pool_size = nn_params[5 + (layer * 2) + 1];
        
        int bias_weight = current_weight + (conv_size * conv_size);
        int next_x = image_x - conv_size;
        int next_y = image_y - conv_size;
        for (int i = 0; i < next_x; i++) {
            for (int j = 0; j < next_y; j++) {

                current_weight = 4;
                tmp = 0.0f;
                for (int k = 0; k < conv_size; k++) {
                    pos = ((i + k) * image_x) + j;
                    for (int l = 0; l < conv_size; l++) {
                        tmp += weights[current_weight] * nodes[pos];
                        current_weight++;
                        pos++;
                    }
                }

                tmp = tmp + weights[bias_weight];

                nodes[(i * next_x) + j] = 1.0f / (1.0f + exp(-tmp));

                bias_weight++;
            }
        }
        image_x = next_x;
        image_y = next_y;
        current_weight = bias_weight;


        next_x = image_x / pool_size;
        next_y = image_y / pool_size;
        for (int i = 0; i < next_x; i++) {
            int next_pos = i * next_x;
            for (int j = 0; j < next_y; j++) {

                tmp = -99.0f;
                int j_pos = j * pool_size;
                for (int k = 0; k < pool_size; k++) {
                    pos = (((i * pool_size) + k) * image_x) + j_pos;
                    for (int l = 0; l < pool_size; l++) {
                        if (tmp < nodes[pos]) tmp = nodes[pos];
                        pos++;
                    }
                }
                nodes[next_pos] = tmp;
                next_pos++;
            }
        }
        image_x = next_x;
        image_y = next_y;

    }



    for (int i = 0; i < fc_size; i++) {
        tmp = 0.0f;
        pos = 0;
        for (int j = 0; j < image_x; j++) {
            pos = (j * image_x);
            for (int k = 0; k < image_y; k++) {
                tmp += nodes[pos] * weights[current_weight];
                current_weight++;
                pos++;
            }
        }
        tmp = tmp + weights[current_weight]; //for bias
        fc_layer[i] = 1.0f / (1.0f + exp(-tmp));

        //printf("fc nodes[%d]: %f\n", i, fc_layer[i]);
        current_weight++;
//        printf("fc_layer[%d]: %f\n", i, fc_layer[i]);
    }


    float sum = 0.0f;
    for (int i = 0; i < n_classes; i++) {
        tmp = 0.0f;
        for (int j = 0; j < fc_size; j++) {
            tmp += fc_layer[j] * weights[current_weight];
            current_weight++;
        }
        tmp = tmp + weights[current_weight]; //for bias
        current_weight++;

        tmp = 1.0f / (1.0f + exp(-tmp));
//        printf("tmp: %lf\n", tmp);

        nodes[i] = tmp;
        //printf("output nodes[%d]: %f\n", i, nodes[i]);
        sum += tmp;
//        printf("sum: %lf\n", sum);
    }

    int max_pos = 0;
    for (int i = 0; i < n_classes; i++) {
        nodes[i] /= sum;
//        printf("output nodes[%d]: %f\n", i, nodes[i]);
        if (image_pos < nn_params[5 + (n_layers * 2) + i]) {
//            printf("class was: %d because group %d was less than class %d\n", i, image_pos, nn_params[5 + (n_layers * 2) + i]);
            output_classifications[image_pos] = log(nodes[i]);
            break;
        }
    }

//    output_classifications[image_pos] = log(nodes[classification]);
}

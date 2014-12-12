/*
 *  Need to load images into constant memory (these won't change)
 *
 *  Need to load weights into constant memory (won't change per run)
 *
 *  Nodes need to be local memory (if there is enougH)
 */

__kernel void add_numbers(__global char *image, __constant float *weights, __local float* nodes, __local float* fc_layer, __global float* output_image) {
    //these need to be input parameters somehow
    int image_x = 16;
    int image_y = 16;

    int grp_sz_x = get_global_size(0);
    int grp_sz_y = get_global_size(1);

    int grp_id_x = get_group_id(0);
    int grp_id_y = get_group_id(1);

    int original_cols = grp_sz_x + image_x;
    int original_rows = grp_sz_y + image_y;

    int conv_size = 4;
    int pool_size = 2;
    int fc_size = 6;
    int n_classes = 2;
    int classification = 0;

    int current_image_pos;
    int pos = 0;
    float tmp;
    for (int i = 0; i < image_x; i++) {
        current_image_pos = ((grp_id_y + i) * original_cols * 3) + (grp_id_x * 3);

        for (int j = 0; j < image_y; j++) {
            tmp = 0.0f;

            tmp  = weights[0] * (image[current_image_pos] / 256.0f);
            tmp += weights[1] * (image[current_image_pos + 1] / 256.0f);
            tmp += weights[2] * (image[current_image_pos + 2] / 256.0f);
            tmp += weights[3];
            tmp = 1.0f / (1.0f + exp(tmp));
            nodes[pos] = tmp;
            pos++;
            current_image_pos += 3;
        }
    }
    int current_weight = 4;

//    for (int layer = 0; layer < layers; layer++) {
        
        int bias_weight = current_weight + (conv_size * conv_size);
        for (int i = 0; i < image_x - conv_size; i++) {
            for (int j = 0; j < image_y - conv_size; j++) {

                current_weight = 4;
                tmp = 0.0f;
                for (int k = 0; k < conv_size; k++) {
                    for (int l = 0; l < conv_size; l++) {
                        pos = ((i + k) * image_x) + (j + l);
                        tmp += weights[current_weight] * nodes[pos];
                        current_weight++;
                    }
                }

                tmp = tmp + weights[bias_weight];

                nodes[(i * (image_x-conv_size)) + j] = 1.0f / (1.0f + exp(tmp));
                bias_weight++;
            }
        }
        image_x -= conv_size;
        image_y -= conv_size;
        current_weight = bias_weight;



        for (int i = 0; i < image_x / pool_size; i++) {
            for (int j = 0; j < image_y / pool_size; j++) {

                tmp = -99.0f;
                for (int k = 0; k < pool_size; k++) {
                    for (int l = 0; l < pool_size; l++) {
                        pos = (((i * pool_size) + k) * image_x) + ((j * pool_size) + l);
                        if (tmp < nodes[pos]) tmp = nodes[pos];
                    }
                }
                nodes[(i * (image_x/pool_size)) + j] = tmp;

            }
        }
        image_x /= pool_size;
        image_y /= pool_size;

//    }


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
        fc_layer[i] = 1.0f / (1.0f + exp(tmp));
        current_weight++;
//        printf("fc_layer[%d]: %f\n", i, fc_layer[i]);
    }

    float sum = 0.0f;
    for (int i = 0; i < n_classes; i++) {
        tmp = 0.0f;
        for (int j = 0; j < fc_size; j++) {
            tmp += fc_layer[j] * current_weight;
            current_weight++;
        }
        tmp = tmp + weights[current_weight]; //for bias
        current_weight++;

        tmp = 1.0f / (1.0f + exp(tmp));
//        printf("tmp: %lf\n", tmp);

        nodes[i] = tmp;
        sum += tmp;
//        printf("sum: %lf\n", sum);
    }

    for (int i = 0; i < n_classes; i++) {
        nodes[i] /= sum;
    }

    //output_image[(grp_id_y * grp_sz_x) + grp_id_x] = sum;
    output_image[(grp_id_y * grp_sz_x) + grp_id_x] = nodes[classification];
}

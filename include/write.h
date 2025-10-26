#ifndef WRITE_H
#define WRITE_H
#include <stdio.h>
#include <stdlib.h>

#include "image.h"

void writeImage(const char* filename, Image image) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%d %d\n", image.width, image.height);
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
            fprintf(file, "%.6f", image.data[i][j]);
            if (j < image.width - 1) {
                fprintf(file, " ");
            }
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

#endif

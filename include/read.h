#ifndef READ_H
#define READ_H
#include <stdio.h>

#include "image.h"

// 從檔案讀取影像資料
static Image readImage(const char* filename) {
    Image image;
    image.name = NULL;  // ?w?]?????
    image.data = NULL;
    image.buffer = NULL;
    image.width = image.height = 0;

    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return image;
    }

    // ??e?P??
    if (fscanf(file, "%d %d", &image.width, &image.height) != 2) {
        fprintf(stderr, "Invalid file format: %s\n", filename);
        fclose(file);
        return image;
    }

    // ?t?m?G???}?C

    image = zerosImage(image.width, image.height, filename);

    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
            if (fscanf(file, "%f", &image.data[i][j]) != 1) {
                fprintf(stderr, "Invalid pixel data at row %d, col %d\n", i, j);
                freeImage(image);
                fclose(file);
                return image;
            }
        }
    }

    fclose(file);
    return image;  //  ???????^??A???? malloc Image ???c
}

#endif

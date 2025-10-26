#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "read.h"
#include "stb_image_write.h"

void writePNG(const char* filename, Image image) {
    int w = image.width, h = image.height;

    uint8_t* buffer = (uint8_t*)malloc(h * w * sizeof(uint8_t));

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            buffer[i * w + j] = (uint8_t)(image.buffer[(h - i - 1) * w + j] * 255);
        }
    }

    stbi_write_png(filename, w, h, 1, buffer, w);
    free(buffer);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <img1.txt> <img2.txt> < ... >\n", argv[0]);
        return 1;
    }

    // 依序處理每個圖像
    for (int i = 1; i < argc; i++) {
        Image img = readImage(argv[i]);

        if (!img.data) {
            fprintf(stderr, "Error: Unable to read image from %s\n", argv[i]);
            continue;
        }

        // 產生輸出檔名 *.txt -> *.png
        int len = strlen(argv[i]);
        char* output = (char*)malloc((len + 1) * sizeof(char));
        strcpy(output, argv[i]);
        output[len - 3] = 'p', output[len - 2] = 'n', output[len - 1] = 'g';

        writePNG(output, img);
        free(output);
        freeImage(img);
    }

    return 0;
}

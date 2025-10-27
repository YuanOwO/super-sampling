#ifndef IMAGE_H
#define IMAGE_H

#include <stdlib.h>
#include <string.h>

typedef struct {
    int width;
    int height;
    float** data;
    float* buffer;  // 儲存在連續的記憶體區塊中
    char* name;
} Image;

// 建立一個全零的影像
static Image zerosImage(int width, int height, const char* name) {
    Image image;
    image.width = width;
    image.height = height;
    if (name) {
        image.name = (char*)malloc((strlen(name) + 1) * sizeof(char));
        strcpy(image.name, name);
    } else {
        image.name = NULL;
    }

    // 連續的記憶體區塊
    image.buffer = (float*)malloc(width * height * sizeof(float));
    memset(image.buffer, 0, width * height * sizeof(float));

    image.data = (float**)malloc(height * sizeof(float*));
    for (int i = 0; i < height; i++)
        image.data[i] = image.buffer + i * width;  // 指向每一列的起始位置

    return image;
}

// 清除 Image 資源
static void freeImage(Image img) {
    if (img.data) free(img.buffer), free(img.data);
    if (img.name) free(img.name);
    img.name = NULL;
    img.data = NULL;
    img.width = img.height = 0;
}

// 轉置影像
static void transposeImage(Image* img) {
    if (!img || !img->data) return;
    Image tmp = zerosImage(img->height, img->width, img->name);

    for (int i = 0; i < img->height; i++)
        for (int j = 0; j < img->width; j++)
            tmp.data[j][i] = img->data[i][j];

    freeImage(*img);  // 釋放原本的記憶體
    *img = tmp;       // 更新 img 為轉置後的影像
}

#endif  // IMAGE_H

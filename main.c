#include <emscripten.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"
#include <stdlib.h>

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    unsigned char* data;
    int width;
    int height;
    int channels;
} ImageResult;

static int img_w, img_h, img_c;

bool isOutOfBounds(int x, int y) {
    return x < 0 || x >= img_w || y < 0 || y >= img_h;
}

void setPixel(uint8_t* img, int x, int y, int c, uint8_t val) {

    if (x < 0 || x > img_w || y < 0 || y > img_h) {
        return;
    }

    img[(y * img_w + x) * img_c + c] = val;

}

int getPixel(uint8_t* img, int x, int y, int c) {

    return img[(y * img_w + x) * img_c + c];
}

int getIndex(int x, int y) {

    return (y * img_w + x) * img_c;

}

int getPxlBrightness(uint8_t* img, int x, int y) {
    if (img_c == 4 && img[(y * img_w + x) * img_c + 3] == 0) {
        return 255;
    }
    int sum = 0;
    for (int c = 0; c < 3; c++) {
        sum += img[(y * img_w + x) * img_c + c];
    }
    return sum / 3;
}

void copyPxl(uint8_t* img, int x, int y, int new_x, int new_y) {
    if (isOutOfBounds(new_x, new_y) || isOutOfBounds(x, y)) {
        return;
    }

    for (int c = 0; c < img_c; c++) {
        setPixel(img, new_x, new_y, c, getPixel(img, x, y, c));
    }
}

void swapPxl(uint8_t* img, int x1, int y1, int x2, int y2) {
    if (isOutOfBounds(x1, y1) || isOutOfBounds(x2, y2)) {
        return;
    }
    for (int c = 0; c < img_c; c++) {
        int tmp = getPixel(img, x1, y1, c);
        setPixel(img, x1, y1, c, getPixel(img, x2, y2, c));
        setPixel(img, x2, y2, c, tmp);
    }
}

void pxlBleed(uint8_t* img, int randOffset, int randChance, int tolerance, int xy_mode, int wrap, int size) {
    int dist, brightness, new_x, new_y;
    // uint8_t* orig_img = malloc(img_w * img_h * img_c);
    // memcpy(orig_img, img, img_w * img_h * img_c);
    for (int y = 0; y < img_h; y += size) {
        for (int x = 0; x < img_w; x += size) {
            // brightness = getPxlBrightness(img, x, y);
            if (getPxlBrightness(img, x, y) < tolerance || rand() % 100 < randChance) {
                dist = rand() % randOffset;
                new_x = x + dist;
                new_y = y + dist;
                if (wrap) {
                    new_x = new_x % img_w;
                    new_y = new_y % img_h;
                } 
                for (int i = 0; i < size; i++) {
                    for (int j = 0; j < size; j++) {
                        if (xy_mode == 0) {
                            copyPxl(img, x + i, y + j, new_x + i, y + j);
                        } else if (xy_mode == 1) {
                            copyPxl(img, x + i, y + j, x + i, new_y + j);
                        } else {
                            copyPxl(img, x + i, y + j, new_x + i, new_y + j);
                        }
                    }
                }
            }
        }
    }
    // free(orig_img);
}

EMSCRIPTEN_KEEPALIVE
ImageResult* processImage(unsigned char* data, int size, int randOffset, int randChance, int tolerance, int xy_mode, int wrap, int size2) {
    if (size2 < 1) {
        size2 = 1;
    }
    int width, height, channels;
    unsigned char* img = stbi_load_from_memory(data, size, &width, &height, &channels, 4);
    img_w = width;
    img_h = height;
    img_c = 4;
    
    if (img == NULL) {
        printf("Error in loading the image\n");
        return NULL;
    }

    pxlBleed(img, randOffset, randChance, tolerance, xy_mode, wrap, size2);

    ImageResult* result = (ImageResult*)malloc(sizeof(ImageResult));
    result->data = img;
    result->width = width;
    result->height = height;
    result->channels = 4;  // We're always using 4 channels (RGBA)

    return result;
}

EMSCRIPTEN_KEEPALIVE
void freeImageResult(ImageResult* result) {
    if (result) {
        if (result->data) {
            free(result->data);
        }
        free(result);
    }
}

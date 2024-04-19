#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

static uint8_t* img;
static int img_w, img_h, img_c;

static inline void loadImg(const char* path) {
    img = stbi_load(path, &img_w, &img_h, &img_c, 0);
    if (!img) {
        fprintf(stderr, "Failed to load image\n");
        exit(1);
    }
}


static inline void saveImg(const char* path) {
    if (!stbi_write_png(path, img_w, img_h, img_c, img, img_w * img_c)) {
        fprintf(stderr, "Failed to save image\n");
        exit(1);
    }
}


static inline int getPixelBrightness(int x, int y) {
    int r = img[(y * img_w + x) * img_c];
    int g = img[(y * img_w + x) * img_c + 1];
    int b = img[(y * img_w + x) * img_c + 2];
    return (r + g + b) / 3;
}


static inline void setPixel(int x, int y, int r, int g, int b) {
    img[(y * img_w + x) * img_c] = r;
    img[(y * img_w + x) * img_c + 1] = g;
    img[(y * img_w + x) * img_c + 2] = b;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
        return 1;
    }
    loadImg(argv[1]);
    printf("Image size: %dx%d\n", img_w, img_h);
    printf("Image channels: %d\n", img_c);
    for (int x = 0; x < img_w; x++) {
        if (getPixelBrightness(x, 100) < 128) {
            for (int y = 0; y < img_h; y++) {
                setPixel(x, y, 255, 255, 255);
            }
        }
        else {
            setPixel(x, 100, 0, 0, 0);
        }
    }
    saveImg("out.png");
    stbi_image_free(img);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

static uint8_t* og_img;
static int img_w, img_h, img_c;

void loadImg(const char* path, int* w, int* h, int* c) {
    og_img = stbi_load(path, w, h, c, 0);
    if (!og_img) {
        fprintf(stderr, "Failed to load image\n");
        exit(1);
    }
}

void saveImg(const char* path, int w, int h, int c, const uint8_t* img) {
    if (!stbi_write_png(path, w, h, c, img, w * c)) {
        fprintf(stderr, "Failed to save image\n");
        exit(1);
    }
}

void setPixel(uint8_t* img, int x, int y, int c, uint8_t val) {

    img[(y * img_w + x) * img_c + c] = val;

}

int getPixel(uint8_t* img, int x, int y, int c) {

    return img[(y * img_w + x) * img_c + c];
}

int getIndex(int x, int y) {

    return (y * img_w + x) * img_c;

}

int getPxlBrightness(uint8_t* img, int x, int y) {
    int sum = 0;
    for (int c = 0; c < 3; c++) {
        sum += img[(y * img_w + x) * img_c + c];
    }
    return sum / 3;
}

void copyPxl(uint8_t* img, int x, int y, int new_x, int new_y) {

    for (int c = 0; c < img_c; c++) {
        setPixel(img, new_x, new_y, c, getPixel(img, x, y, c));
    }
}

void swapPxl(uint8_t* img, int x1, int y1, int x2, int y2) {
    for (int c = 0; c < img_c; c++) {
        int tmp = getPixel(img, x1, y1, c);
        setPixel(img, x1, y1, c, getPixel(img, x2, y2, c));
        setPixel(img, x2, y2, c, tmp);
    }
}

void test(uint8_t* img, int randOffset, int randChance, int tolerance) {
    int dist, brightness;
    uint8_t* orig_img = malloc(img_w * img_h * img_c);
    memcpy(orig_img, img, img_w * img_h * img_c);
    for (int y = 0; y < img_h; y++) {
        for (int x = 0; x < img_w; x++) {
            brightness = getPxlBrightness(img, x, y);
            if (brightness < tolerance || rand() % 100 < randChance) {
                dist = rand() % randOffset;
                if (y * img_w + x + dist < img_h * img_w) {
                    copyPxl(img, x, y, x + dist, y);
                    // swapPxl(img, x, y, x + dist, y);
                }
            }
        }
    }
    free(orig_img);
}



int main(int argc, char** argv) {
    int opt;
    struct {
        int iterations;
        int offset;
        int tol;
        int frame_rate;
        int randChance;
        int help;
    } arguments = {50, 2, 128, 20, 10};

    while ((opt = getopt(argc, argv, "i:o:t:f:r:h")) != -1) {
        switch (opt) {
            case 'i':
                arguments.iterations = atoi(optarg);
                break;
            case 'o':
                arguments.offset = atoi(optarg);
                break;
            case 't':
                arguments.tol = atoi(optarg);
                break;
            case 'f':
                arguments.frame_rate = atoi(optarg);
                break;
            case 'r':
                arguments.randChance = atoi(optarg);
                break;
            case 'h':
                arguments.help = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-i iterations] [-o offset] [-t tolerance] [-f frame_rate] [-r randChance] image\n", argv[0]);
                exit(1);
        }
    }

    if (arguments.help) {
        printf("Usage: %s [-i iterations] [-o offset] [-t tolerance] [-f frame_rate] [-r randChance] image\n", argv[0]);
        exit(0);
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected image path\n");
        exit(1);
    }

    loadImg(argv[optind], &img_w, &img_h, &img_c);

    printf("Loaded image with dimensions %dx%d and %d channels\n", img_w, img_h, img_c);
    printf("Total pixels: %d\n", img_w * img_h);

    uint8_t* mod_img =  malloc(img_w * img_h * img_c);
    memcpy(mod_img, og_img, img_w * img_h * img_c);
    for (int i = 0; i < arguments.iterations; i++) {
        test(mod_img, arguments.offset, arguments.randChance, arguments.tol);
        srand(i);
        printf("Test %d\n", i);
        char filename[50];
        strcpy(filename, "./img/");
        char num[5];
        sprintf(num, "%04d", i);
        strcat(filename, num);
        strcat(filename, ".png");
        saveImg(filename, img_w, img_h, img_c, mod_img);
    }
    stbi_image_free(og_img);

    char cmd[500];
    // sprintf(cmd, "ffmpeg -y -framerate %d -i ./img/%%04d.png -pix_fmt rgb8 -loop 0 -filter_complex \"scale=trunc(iw/2)*2:trunc(ih/2)*2:flags=lanczos[x];[x]split[x1][x2];[x1]palettegen[p];[x2][p]paletteuse\" ./out/output.gif", arguments.frame_rate);
    sprintf(cmd, "ffmpeg -y -framerate %d -i ./img/%%04d.png -filter_complex \"format=rgba,split[x1][x2];[x1]palettegen[p];[x2][p]paletteuse\" -loop 0 ./out/output.gif", arguments.frame_rate);
    system(cmd);

    return 0;
}

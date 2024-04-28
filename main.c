#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

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

bool isOutOfBounds(int x, int y) {
    return x < 0 || x >= img_w || y < 0 || y >= img_h;
}

void pxlBleed(uint8_t* img, int randOffset, int randChance, int tolerance, int xy_mode) {
    int dist, brightness;
    uint8_t* orig_img = malloc(img_w * img_h * img_c);
    memcpy(orig_img, img, img_w * img_h * img_c);
    for (int y = 0; y < img_h; y++) {
        for (int x = 0; x < img_w; x++) {
            brightness = getPxlBrightness(img, x, y);
            if (brightness < tolerance || rand() % 100 < randChance) {
                dist = rand() % randOffset;
                if (xy_mode == 0) {
                    copyPxl(img, x, y, (x + dist) % img_w, y);
                } else if (xy_mode == 1) {
                    copyPxl(img, x, y, x, (y + dist) % img_h);
                } else {
                    copyPxl(img, x, y, (x + dist) % img_w, (y + dist) % img_h);
                }
            }
        }
    }
    free(orig_img);
}

void pxlDiffuse(uint8_t* img, int randOffset, int randChance, int tolerance, int xy_mode) {
    int dist, brightness;
    uint8_t* orig_img = malloc(img_w * img_h * img_c);
    memcpy(orig_img, img, img_w * img_h * img_c);
    for (int y = 0; y < img_h; y++) {
        for (int x = 0; x < img_w; x++) {
            brightness = getPxlBrightness(img, x, y);
            if (brightness < tolerance || rand() % 100 < randChance) {
                dist = rand() % randOffset;
                if (xy_mode == 0) {
                    swapPxl(img, x, y, (x + dist) % img_w, y);
                } else if (xy_mode == 1) {
                    swapPxl(img, x, y, x, (y + dist) % img_h);
                } else {
                    swapPxl(img, x, y, (x + dist) % img_w, (y + dist) % img_h);
                }
            }
        }
    }
    free(orig_img);
}


bool endsWith(const char* str, const char* suffix) {
    if (!str || !suffix) {
        return 0;
    }
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) {
        return 0;
    }
    return 0 == strncmp(str + str_len - suffix_len, suffix, suffix_len);
}


bool isint(char* str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    return true;
}


int main(int argc, char** argv) {
    int opt;
    struct {
        int iterations;
        int offset;
        int tol;
        int frame_rate;
        int randChance;
        char *mode;
        int xy_mode;
        char *output;
        char *input;
    } arguments = {50, 2, 128, 20, 10, NULL, 2, NULL, NULL};

    while ((opt = getopt(argc, argv, "I:i:o:O:t:f:r:m:hxy")) != -1) {
        switch (opt) {
            case 'I':
                if (!isint(optarg)) {
                    fprintf(stderr, "Invalid number of iterations\n");
                    exit(1);
                }
                arguments.iterations = atoi(optarg);
                break;
            case 'i':
                arguments.input = optarg;
                break;
            case 'O':
                if (!isint(optarg)) {
                    fprintf(stderr, "Invalid offset\n");
                    exit(1);
                }
                arguments.offset = atoi(optarg);
                break;
            case 'o':
                arguments.output = optarg;
                break;
            case 't':
                if (!isint(optarg)) {
                    fprintf(stderr, "Invalid tolerance\n");
                    exit(1);
                }
                arguments.tol = atoi(optarg);
                break;
            case 'f':
                if (!isint(optarg)) {
                    fprintf(stderr, "Invalid frame rate\n");
                    exit(1);
                }
                arguments.frame_rate = atoi(optarg);
                break;
            case 'r':
                if (!isint(optarg)) {
                    fprintf(stderr, "Invalid randChance\n");
                    exit(1);
                }
                arguments.randChance = atoi(optarg);
                break;
            case 'm':
                arguments.mode = optarg;
                break;
            case 'x':
                arguments.xy_mode = 0;
                break;
            case 'y':
                arguments.xy_mode = 1;
                break;
            case 'h':
                printf("Usage: %s [-i iterations] [-o offset] [-t tolerance] [-f frame_rate] [-r randChance] [-m mode] [-xy] image\n", argv[0]);
                printf("Options:\n"
                        "  -I iterations: Number of frames to generate (default 50)\n"
                        "  -O offset: Maximum pixel offset (default 2)\n"
                        "  -t tolerance: Minimum brightness to trigger effect (default 128)\n"
                        "  -f frame_rate: Frame rate of output gif (default 20)\n"
                        "  -r randChance: Chance of effect happening (default 10)\n"
                        "  -m mode: Effect mode bleed/diffuse/wind/haze (default bleed)\n"
                        "  -x: Only offset x axis\n"
                        "  -y: Only offset y axis\n"
                        "  -o output: Output file (default output.gif)\n"
                        "  -i input: Input file\n"
                        "  -h: Show help\n"
                        "  image: Path to image\n"
                );
                exit(0);
            default:
                fprintf(stderr, "Usage: %s [-i iterations] [-o offset] [-t tolerance] [-f frame_rate] [-r randChance] [-m mode] [-xy] image\n", argv[0]);
                exit(1);
        }
    }

    if (optind >= argc && arguments.input == NULL) {
        fprintf(stderr, "Expected input image path\n");
        exit(1);
    }

    struct dirent *de;
    DIR *dr = opendir("./img");
    if (dr == NULL) {
        mkdir("./img", 0777);
    }
    else {
        while ((de = readdir(dr)) != NULL) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                continue;
            }
            char path[50];
            strcpy(path, "./img/");
            strcat(path, de->d_name);
            remove(path);
        }
        closedir(dr);
    }

    if (arguments.input) {
        loadImg(arguments.input, &img_w, &img_h, &img_c);
    }
    else {
        loadImg(argv[optind], &img_w, &img_h, &img_c);
    }

    printf("Loaded image with dimensions %dx%d and %d channels\n", img_w, img_h, img_c);

    uint8_t* mod_img =  malloc(img_w * img_h * img_c);
    memcpy(mod_img, og_img, img_w * img_h * img_c);

    char* progress = "Processing frame %d/%d";

    if (arguments.output == NULL) {
        arguments.output = "output.gif";
    }

    clock_t start = clock();

    for (int i = 0; i < arguments.iterations; i++) {
        if (arguments.mode == NULL || strcmp(arguments.mode, "bleed") == 0 || strcmp(arguments.mode, "wind") == 0)
            pxlBleed(mod_img, arguments.offset, arguments.randChance, arguments.tol, arguments.xy_mode);
        else if (strcmp(arguments.mode, "diffuse") == 0 || strcmp(arguments.mode, "haze") == 0)
            pxlDiffuse(mod_img, arguments.offset, arguments.randChance, arguments.tol, arguments.xy_mode);

        srand(i);

        printf("\r");
        printf(progress, i, arguments.iterations);

        char filename[50];
        strcpy(filename, "./img/");
        char num[5];
        sprintf(num, "%04d", i);
        strcat(filename, num);
        strcat(filename, ".png");
        saveImg(filename, img_w, img_h, img_c, mod_img);

        if (arguments.mode != NULL && (strcmp(arguments.mode, "wind") == 0 || strcmp(arguments.mode, "haze") == 0))
            memcpy(mod_img, og_img, img_w * img_h * img_c);
    }

    clock_t end = clock();

    printf("\n\nTime taken: %f seconds\n\n", (double)(end - start) / CLOCKS_PER_SEC);

    stbi_image_free(og_img);

    // if output ends with .gif
    if (endsWith(arguments.output, ".gif")) {
        char cmd[500];
        sprintf(cmd, "ffmpeg -y -framerate %d -i ./img/%%04d.png -filter_complex \"format=rgba,split[x1][x2];[x1]palettegen[p];[x2][p]paletteuse\" -loop 0 %s", arguments.frame_rate, arguments.output);
        system(cmd);
        return 0;
    }
    else if (endsWith(arguments.output, ".png") || endsWith(arguments.output, ".jpg") || endsWith(arguments.output, ".jpeg")) {
        saveImg(arguments.output, img_w, img_h, img_c, mod_img);
    }
    else {
        fprintf(stderr, "Invalid output file type\n");
        exit(1);
    }

    return 0;
}

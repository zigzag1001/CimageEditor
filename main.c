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

struct arguments {
    int iterations;
    int offset;
    int tol;
    int frame_rate;
    int randChance;
    char *mode;
    int xy_mode;
    char *output;
    char *input;
    int animate_iters;
    int wrap;
    int size;
};

typedef struct arguments Arguments;

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
    uint8_t* orig_img = malloc(img_w * img_h * img_c);
    memcpy(orig_img, img, img_w * img_h * img_c);
    for (int y = 0; y < img_h; y += size) {
        for (int x = 0; x < img_w; x += size) {
            brightness = getPxlBrightness(img, x, y);
            if (brightness < tolerance || rand() % 100 < randChance) {
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
    free(orig_img);
}

void pxlDiffuse(uint8_t* img, int randOffset, int randChance, int tolerance, int xy_mode, int wrap, int size) {
    int dist, brightness, new_x, new_y;
    uint8_t* orig_img = malloc(img_w * img_h * img_c);
    memcpy(orig_img, img, img_w * img_h * img_c);
    for (int y = 0; y < img_h; y += size) {
        for (int x = 0; x < img_w; x += size) {
            brightness = getPxlBrightness(img, x, y);
            if (brightness < tolerance || rand() % 100 < randChance) {
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
                            swapPxl(img, x + i, y + j, new_x + i, y + j);
                        } else if (xy_mode == 1) {
                            swapPxl(img, x + i, y + j, x + i, new_y + j);
                        } else {
                            swapPxl(img, x + i, y + j, new_x + i, new_y + j);
                        }
                    }
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


void cleanDir(const char* path) {
    struct dirent *de;
    DIR *dr = opendir(path);
    if (dr == NULL) {
        mkdir(path, 0777);
        return;
    }
    else {
        while ((de = readdir(dr)) != NULL) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                continue;
            }
            char file[50];
            strcpy(file, path);
            strcat(file, "/");
            strcat(file, de->d_name);
            remove(file);
        }
        closedir(dr);
    }
}


void modify(Arguments *arguments) {

    loadImg(arguments->input, &img_w, &img_h, &img_c);

    printf("Loaded image with dimensions %dx%d and %d channels\n", img_w, img_h, img_c);

    // Initialize modified image
    uint8_t* mod_img =  malloc(img_w * img_h * img_c);
    memcpy(mod_img, og_img, img_w * img_h * img_c);

    for (int i = 0; i < arguments->iterations; i++) {
        if (arguments->mode == NULL || strcmp(arguments->mode, "bleed") == 0 || strcmp(arguments->mode, "wind") == 0)
            pxlBleed(mod_img, arguments->offset, arguments->randChance, arguments->tol, arguments->xy_mode, arguments->wrap, arguments->size);
        else if (strcmp(arguments->mode, "diffuse") == 0 || strcmp(arguments->mode, "haze") == 0)
            pxlDiffuse(mod_img, arguments->offset, arguments->randChance, arguments->tol, arguments->xy_mode, arguments->wrap, arguments->size);

        srand(i+5);

        printf("\r");
        printf("Processing frame %d/%d", i, arguments->iterations);

        if (endsWith(arguments->output, ".gif") || endsWith(arguments->output, ".mp4")) {
            char filename[50];
            strcpy(filename, "./img/");
            char num[5];
            sprintf(num, "%04d", i);
            strcat(filename, num);
            strcat(filename, ".png");
            saveImg(filename, img_w, img_h, img_c, mod_img);
        }

        if (arguments->mode != NULL && (strcmp(arguments->mode, "wind") == 0 || strcmp(arguments->mode, "haze") == 0))
            memcpy(mod_img, og_img, img_w * img_h * img_c);
    }
    if (!endsWith(arguments->output, ".gif") && !endsWith(arguments->output, ".mp4"))
        saveImg(arguments->output, img_w, img_h, img_c, mod_img);

    free(mod_img);
    stbi_image_free(og_img);
}

int main(int argc, char** argv) {
    int opt;


    Arguments arguments;

    Arguments *args = &arguments;

    arguments.iterations = 50;
    arguments.offset = 2;
    arguments.tol = 128;
    arguments.frame_rate = 20;
    arguments.randChance = 10;
    arguments.mode = NULL;
    arguments.xy_mode = 2;
    arguments.output = NULL;
    arguments.input = NULL;
    arguments.animate_iters = 0;
    arguments.wrap = 0;
    arguments.size = 1;


    // Parse arguments
    while ((opt = getopt(argc, argv, "I:i:o:O:t:f:r:m:a:s:hxyw")) != -1) {
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
            case 's':
                if (!isint(optarg)) {
                    fprintf(stderr, "Invalid size\n");
                    exit(1);
                }
                arguments.size = atoi(optarg);
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
            case 'w':
                arguments.wrap = 1;
                break;
            case 'a':
                if (!isint(optarg)) {
                    exit(1);
                }
                arguments.animate_iters = atoi(optarg);
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
                        "  -a animate_iters: animate number of iters for each frame of video/gif (default 0)\n"
                        "  -s size: Size of pixel block (default 1)\n"
                        "  -w: Wrap around image\n"
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

    // Check if input image is provided
    if (optind >= argc && arguments.input == NULL) {
        fprintf(stderr, "Expected input image path\n");
        exit(1);
    }

    // handle directory
    cleanDir("./img");

    // set input image
    if (!arguments.input) {
        arguments.input = argv[optind];
    }


    // Set default output file
    if (arguments.output == NULL) {
        arguments.output = "output.gif";
    }

    char og_output[50];
    strcpy(og_output, arguments.output);
    char og_input[50];
    strcpy(og_input, arguments.input);

    clock_t start = clock();

    if (endsWith(arguments.input, ".png") || endsWith(arguments.input, ".jpg") || endsWith(arguments.input, ".jpeg")) {
        modify(args);
    }
    else if (endsWith(arguments.input, ".gif") || endsWith(arguments.input, ".mp4")) {
        cleanDir("./frames");
        char cmd[500];
        sprintf(cmd, "ffmpeg -i %s ./frames/%%04d.png", arguments.input);
        system(cmd);
        struct dirent *de;
        DIR *dr = opendir("./frames");
        if (dr == NULL) {
            fprintf(stderr, "Failed to load frames\n");
            exit(1);
        }
        char path[50];
        char processed_path[50];
        while ((de = readdir(dr)) != NULL) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                continue;
            }
            strcpy(path, "./frames/");
            strcat(path, de->d_name);
            strcpy(processed_path, "./img/");
            strcat(processed_path, de->d_name);
            arguments.input = path;
            arguments.output = processed_path;
            modify(args);
            printf("\r");
            printf("Processed frame %s\n", de->d_name);
            arguments.iterations += arguments.animate_iters;
        }
        closedir(dr);
    }
    else {
        fprintf(stderr, "Invalid input file type\n");
        exit(1);
    }

    clock_t end = clock();

    printf("\n\nTime taken: %f seconds\n\n", (double)(end - start) / CLOCKS_PER_SEC);

    // if output ends with .gif then convert all images to gif
    if (endsWith(og_output, ".gif")) {
        char cmd[500];
        char img_path[50];
        strcpy(img_path, "./img/%04d.png");
        strcpy(cmd, "ffmpeg -y -framerate ");
        char fr[5];
        sprintf(fr, "%d", arguments.frame_rate);
        strcat(cmd, fr);
        strcat(cmd, " -i ");
        strcat(cmd, img_path);
        strcat(cmd, " -filter_complex \"format=rgba,split[x1][x2];[x1]palettegen[p];[x2][p]paletteuse\" -loop 0 ");
        strcat(cmd, og_output);
        system(cmd);
    }
    else if (endsWith(og_output, ".mp4")) {
        char cmd[500];
        char img_path[50];
        strcpy(img_path, "./img/%04d.png");
        strcpy(cmd, "ffmpeg -y -framerate ");
        char fr[5];
        sprintf(fr, "%d", arguments.frame_rate);
        strcat(cmd, fr);
        strcat(cmd, " -i ");
        strcat(cmd, img_path);
        strcat(cmd, " -c:v libx264 -pix_fmt yuv420p ");
        strcat(cmd, og_output);
        system(cmd);
    }
    else if (endsWith(og_output, ".png") || endsWith(og_output, ".jpg") || endsWith(og_output, ".jpeg")) {
        printf("Saved image to %s\n", og_output);
    }
    else {
        fprintf(stderr, "Invalid output file type\n");
        exit(1);
    }

    return 0;
}

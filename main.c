#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

static uint8_t *img; // store the loaded image in a global variable
static uint8_t *img_copy; // store a copy of the loaded image

static inline void loadImg(char *filename, int *width, int *height, int *channels)
{
    img = stbi_load(filename, width, height, channels, 0);
    if (img == NULL)
    {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", *width, *height, *channels);

    // allocate memory for the copy of the image and copy the original image into it
    img_copy = (uint8_t *) malloc(*width * *height * *channels);
    memcpy(img_copy, img, *width * *height * *channels);
}

static inline void pxlWind(char filename[], int width, int height, int channels, int rand_len_modifier)
{
    // modify the copy of the image instead of the original image
    uint8_t *img_mod = (uint8_t *) malloc(width * height * channels * 4);
    memcpy(img_mod, img_copy, width * height * channels);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * width * channels + x * channels;

            int brightness = (img_mod[index] + img_mod[index + 1] + img_mod[index + 2]) / 3;

            if (brightness < 128 || rand() % 100 < 10)
            {
                int rand_len = rand() % rand_len_modifier;
                for (int i = 0; i < rand_len; i++)
                {
                    int j = x + i;
                    int old_index = y * width * channels + j * channels;
                    int new_index = y * width * channels + (j + rand_len) * channels;
                    img_mod[new_index] = img_mod[old_index];
                    img_mod[new_index + 1] = img_mod[old_index + 1];
                    img_mod[new_index + 2] = img_mod[old_index + 2];
                }
            }
        }
    }

    stbi_write_png(filename, width, height, channels, img_mod, width * channels);

    free(img_mod);
}

static inline void pxlBleed(char filename[], int width, int height, int channels, int rand_len_modifier)
{
    for (int y = 0; y < height-1; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * width * channels + x * channels;

            int brightness = (img[index] + img[index + 1] + img[index + 2]) / 3;

            if (brightness < 128 || rand() % 100 < 10)
            {
                int rand_len = rand() % rand_len_modifier;
                for (int i = 0; i < rand_len; i++)
                {
                    int j = x + i;
                    int old_index = y * width * channels + j * channels;
                    int new_index = y * width * channels + (j + rand_len) * channels;
                    img[new_index] = img[old_index];
                    img[new_index + 1] = img[old_index + 1];
                    img[new_index + 2] = img[old_index + 2];
                }
            }
        }
    }

    stbi_write_png(filename, width, height, channels, img, width * channels);
}

int main(int argc, char **argv)
{
    char *filename = argv[1];
    int n = 1;
    if (argc > 3)
        sscanf(argv[3], "%d", &n);
    printf("%d", n);
    int width, height, channels;
    loadImg(filename, &width, &height, &channels);
    int rand_len_mod_arg = 0;
    if (argc > 4)
        sscanf(argv[4], "%d", &rand_len_mod_arg);
    printf("%d", rand_len_mod_arg);
    char path[50];
    if (strcmp(argv[2], "pxlWind") == 0) {
        for (int i = 0; i < n; i++)
        {
            sprintf(path, "./img/%04d.png", i);
            if (rand_len_mod_arg == 0)
                rand_len_mod_arg = 4;
            pxlWind(path, width, height, channels, rand_len_mod_arg);
        }
    } else if (strcmp(argv[2], "pxlBleed") == 0) {
        for (int i = 0; i < n; i++)
        {
            sprintf(path, "./img/%04d.png", i);
            if (rand_len_mod_arg == 0)
                rand_len_mod_arg = 3;
            pxlBleed(path, width, height, channels, rand_len_mod_arg);
        }
    } else {
        printf("Unknown effect %s", argv[2]);
        exit(1);
    }
    int frame_rate = 13;
    if (argc > 5)
        sscanf(argv[5], "%d", &frame_rate);

    char cmd[100];
    sprintf(cmd, "ffmpeg -y -framerate %d -i ./img/%%04d.png -pix_fmt rgb8 -loop 0 -filter_complex \"scale=trunc(iw/2)*2:trunc(ih/2)*2:flags=lanczos[x];[x]split[x1][x2];[x1]palettegen[p];[x2][p]paletteuse\" ./out/output.gif", frame_rate);

    // make gif
    system(cmd);

    // delete images in img folder without using system
    char path2[50];
    for (int i = 0; i < n; i++)
    {
        sprintf(path2, "./img/%04d.png", i);
        remove(path2);
    }

    stbi_image_free(img);
    return 0;
}

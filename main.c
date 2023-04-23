#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

int makeImg(char filename[])
{
    int width, height, channels;
    unsigned char *img = stbi_load("img.jpg", &width, &height, &channels, 0);
    if (img == NULL)
    {
        printf("Error in loading the image\n");
        exit(1);
    }

    srand(time(NULL)); // Seed the random number generator

    for (int y = 0; y < height - 15; y++)
    {
        for (int x = 0; x < width - 15; x++)
        {
            int index = y * width * channels + x * channels;

            int red = img[index];
            int green = img[index + 1];
            int blue = img[index + 2];

            float brightness = 0.2126 * red + 0.7152 * green + 0.0722 * blue;

            if (brightness < 128 || rand() % 100 < 10)
            {
                int rand_len = rand() % 4;
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

    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

    stbi_write_png(filename, width, height, channels, img, width * channels);

    stbi_image_free(img);
}

int main(void)
{
    char path[50];
    for (int i = 0; i < 15; i++) {
        sprintf(path, "./img/%04d.png", i);
        makeImg(path);
    }
    return 0;
}
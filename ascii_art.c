/*
Simple Screenshot/Image -> ASCII art converter in C
Requires: stb_image (https://github.com/nothings/stb)

Usage:
  1. Download stb_image.h and place it next to this file:
     https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
  2. Compile:
     gcc -O2 ascii_art.c -o ascii_art -lm
  3. Run:
     ./ascii_art input.png         # default width 120
     ./ascii_art input.jpg 80      # set output width to 80 chars
     ./ascii_art input.jpg 100 inv # invert brightness mapping

Notes:
 - The program prints ASCII art to stdout. Redirect to a file if you want:
     ./ascii_art screen.png 120 > out.txt
 - The code uses a simple block-averaging when downscaling to preserve detail.
 - Character aspect correction factor chosen to match typical terminal char proportions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* stb_image: single-file image loader. Download stb_image.h into the same folder.
   URL: https://github.com/nothings/stb/blob/master/stb_image.h
*/
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* ASCII ramps. Dark -> Light */
static const char *RAMP_DEFAULT = "@%#*+=-:. ";
static const char *RAMP_INVERT  = " .:-=+*#%@";

/* Clamp helper */
static inline int clampi(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }

/* Compute a character from a normalized luminance in [0,1] */
static char luminance_to_char(float lum, const char *ramp) {
    int len = (int)strlen(ramp);
    /* lum 0->dark, 1->light. Map to ramp indices 0..len-1 */
    int idx = (int)floorf(lum * (len - 1) + 0.5f);
    idx = clampi(idx, 0, len - 1);
    return ramp[idx];
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
            "Usage: %s <image> [width] [inv]\n"
            "  image : path to PNG/JPEG/BMP/GIF/TGA/WebP etc\n"
            "  width : desired output width in characters (default 120)\n"
            "  inv   : if present (e.g. 'inv'), invert ASCII brightness mapping\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int target_width = 120;
    int invert = 0;
    if (argc >= 3) {
        target_width = atoi(argv[2]);
        if (target_width <= 0) target_width = 120;
    }
    if (argc >= 4) {
        if (strcmp(argv[3], "inv") == 0 || strcmp(argv[3], "invert") == 0) invert = 1;
    }

    int w, h, channels;
    unsigned char *img = stbi_load(filename, &w, &h, &channels, 0);
    if (!img) {
        fprintf(stderr, "Failed to load image '%s' (stb_image error)\n", filename);
        return 2;
    }

    /* If channels < 3, we will treat it as grayscale by repeating the single channel */
    if (channels < 3) channels = 1;

    /* Aspect ratio correction: characters are typically ~2x taller than wide.
       We use a factor to scale height appropriately so the ASCII looks proportionate.
       You can tweak CHAR_ASPECT to taste: 0.5..0.6 are common.
    */
    const float CHAR_ASPECT = 0.55f; /* height/width ratio of a character cell */

    int out_w = target_width;
    int out_h = (int)roundf((float)h * ((float)out_w / (float)w) * CHAR_ASPECT);
    if (out_h < 1) out_h = 1;

    const char *ramp = invert ? RAMP_INVERT : RAMP_DEFAULT;

    /* For each output cell, compute average luminance over the corresponding source rectangle */
    for (int oy = 0; oy < out_h; ++oy) {
        /* source y-range */
        float sy0f = (float)oy * (float)h / (float)out_h;
        float sy1f = (float)(oy + 1) * (float)h / (float)out_h;
        int sy0 = (int)floorf(sy0f);
        int sy1 = (int)ceilf(sy1f);
        sy0 = clampi(sy0, 0, h);
        sy1 = clampi(sy1, 0, h);

        for (int ox = 0; ox < out_w; ++ox) {
            float sx0f = (float)ox * (float)w / (float)out_w;
            float sx1f = (float)(ox + 1) * (float)w / (float)out_w;
            int sx0 = (int)floorf(sx0f);
            int sx1 = (int)ceilf(sx1f);
            sx0 = clampi(sx0, 0, w);
            sx1 = clampi(sx1, 0, w);

            double sum_lum = 0.0;
            int count = 0;
            for (int sy = sy0; sy < sy1; ++sy) {
                for (int sx = sx0; sx < sx1; ++sx) {
                    int idx = (sy * w + sx) * (channels == 1 ? 1 : channels);
                    unsigned char r, g, b;
                    if (channels == 1) {
                        r = g = b = img[idx];
                    } else {
                        r = img[idx + 0];
                        g = img[idx + 1];
                        b = img[idx + 2];
                    }
                    float lum = (0.2126f * r + 0.7152f * g + 0.0722f * b) / 255.0f;
                    sum_lum += lum;
                    ++count;
                }
            }
            float avg_lum = (count > 0) ? (float)(sum_lum / count) : 0.0f;
            /* avg_lum is 0 (dark) -> 1 (bright). ramp string is ordered dark->light */
            char ch = luminance_to_char(avg_lum, ramp);
            putchar(ch);
        }
        putchar('\n');
    }

    stbi_image_free(img);
    return 0;
}
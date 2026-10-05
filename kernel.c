#include "loader.h"
#include <stdlib.h>
#include <string.h>

/** Returns p1 with each channel multiplied by scalar. */
struct pixel mul(struct pixel p1, float scalar) {
    return (struct pixel){r: p1.r * scalar, g: p1.g * scalar, b: p1.b * scalar};
}
/** Returns the channel-wise sum of p1 and p2. */
struct pixel add(struct pixel p1, struct pixel p2) {
    return (struct pixel){r: p1.r + p2.r, g: p1.g + p2.g, b: p1.b + p2.b};
}

/**
 * Applies a square kernel to an image (cross-correlation).
 *
 * Produces a new image where each output pixel is the weighted sum of
 * the ksize x ksize neighborhood centered on the corresponding input
 * pixel, multiplied by normalize. The kernel is applied as-is (not
 * flipped), so this is technically cross-correlation; the result is
 * identical to convolution for symmetric kernels.
 *
 * The input img is padded so that kernel operations that fall outside of the 
 * original image are multiplied by a black pixel (zero padding).
 *
 * img        Source image. Not modified.
 * kernel     Kernel weights in row-major order, containing ksize * ksize elements.
 * ksize      Width and height of the kernel. Should be odd
 * normalize  Scale factor applied to each weighted sum
 *                       (e.g., 1.0f / 9 for a 3x3 box blur).
 *
 * Returns a pointer to a newly allocated image with the same dimensions as img.
 *
 */
struct image* apply_kernel(struct image* img, int* kernel, int ksize, float normalize) {
    struct image* result = malloc(sizeof(struct image));
    result->width = img->width;
    result->height = img->height;
    result->pixels = malloc(result->width * result->height * sizeof(struct pixel));

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            float sum_r = 0, sum_g = 0, sum_b = 0;
            for (int ky = 0; ky < ksize; ky++) {
                for (int kx = 0; kx < ksize; kx++) {
                    int px = x + kx - ksize / 2;
                    int py = y + ky - ksize / 2;
                    if (px >= 0 && px < img->width && py >= 0 && py < img->height) {
                        struct pixel p = img->pixels[py * img->width + px];
                        int weight = kernel[ky * ksize + kx];
                        sum_r += p.r * weight;
                        sum_g += p.g * weight;
                        sum_b += p.b * weight;
                    }
                }
            }
            result->pixels[y * result->width + x] = (struct pixel){r: sum_r * normalize, g: sum_g * normalize, b: sum_b * normalize};
        }
    }
    return result;
}

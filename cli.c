#include "kernel.h"
#include <string.h>
#include <sys/mman.h>

static void unmap_image(struct image* image) {
    size_t mapping_size = sizeof(struct image) +
        (size_t)image->width * (size_t)image->height * sizeof(struct pixel);
    munmap((char*)image->pixels - sizeof(struct image), mapping_size);
}

int generate_pagefault() {
    int* pagefault = NULL;
    *pagefault = 0;
    return 0;
}

//Implement a parser in cli such that you can run your kernel with: 
//./cli kernel images/sky.bmp 640 426 out.bmp
int main(int argc, char** argv){
    if(argc != 6) {
        printf("Incorrect number of arguments. Expected: ./cli <MODE=kernel|mmap|convert|uconvert|fault> <input_image> <width> <height> <output_image_path>\n");
        return -1;
    }

    char* mode = argv[1];
    char* input_filepath = argv[2];
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);
    char* output_filepath = argv[5];

    // You can expect argv[1] to be the mode
    // You can expect argv[2] to be the filepath
    // You can expect argv[3] to be the integer width
    // You can expect argv[4] to be the integer height
    // You can expect argv[5] to be the output filepath.

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Image dimensions must be positive\n");
        return 1;
    }

    if (strcmp(mode, "convert") == 0) {
        struct image img = {.pixels = NULL, .width = width, .height = height};
        if (loadimage(input_filepath, &img) != 0) {
            fprintf(stderr, "Failed to load input image: %s\n", input_filepath);
            return 1;
        }

        int status = saveimage_mmap(output_filepath, &img);
        free(img.pixels);
        return status;
    }

    if (strcmp(mode, "uconvert") == 0) {
        struct image img = {.pixels = NULL, .width = width, .height = height};
        if (loadimage_mmap(input_filepath, &img) != 0) {
            fprintf(stderr, "Failed to map input image: %s\n", input_filepath);
            return 1;
        }

        int status = saveimage(output_filepath, &img);
        unmap_image(&img);
        return status;
    }

    if (strcmp(mode, "kernel") == 0 || strcmp(mode, "mmap") == 0) {
        int use_mmap = strcmp(mode, "mmap") == 0;
        struct image img = {.pixels = NULL, .width = width, .height = height};
        int load_status = use_mmap
            ? loadimage_mmap(input_filepath, &img)
            : loadimage(input_filepath, &img);
        if (load_status != 0) {
            fprintf(stderr, "Failed to load input image: %s\n", input_filepath);
            return 1;
        }

        int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}};

        struct image* result = apply_kernel(&img, (int*)kernel, 3, 1.0f/9);
        if (result == NULL) {
            if (use_mmap) unmap_image(&img);
            else free(img.pixels);
            return 1;
        }

        int status = use_mmap
            ? saveimage_mmap(output_filepath, result)
            : saveimage(output_filepath, result);
        free(result->pixels);
        free(result);
        if (use_mmap) unmap_image(&img);
        else free(img.pixels);
        return status;
    }

    fprintf(stderr, "Unsupported mode: %s\n", mode);
    return 1;
}

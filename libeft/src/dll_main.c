#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
 
/* this value is in the first 8 bytes of the header, and seems to always be the same */
#define EFT_MAGIC_NUM 1103806595072

/* 
 * Notes on EFT files:
 * - The first 8 bytes are a magic number (EFT_MAGIC_NUM)
 * - The next 8 bytes are two (padded?) integers
 *   that set what width and height the texture will be.
 *   (Height is first 4 bytes, Width is second 4 bytes)
 * - Any other data past the first 16 bytes is either 
 *   blank or seems to tell the game engine how to piece
 *   the file together. 
 *   This information isn't really known yet, so certain 
 *   EFTs come out scrambled when converted.
 * - 0x410 is where the DXT1 texture data itself starts, 
 *   and the file terminates with the last encoded block.
 */
struct eft_dimensions_table_entry {
    char code;
    uint32_t actualSize;
};

struct eft_dimensions_table_entry image_size_table[16] = {
    {0x1, 512}, 
    {0x2, 1024},
    {0x3, 1536},
    {0x4, 2048},
    {0x5, 2560},
    {0x6, 3072},
    {0x7, 3584}, 
    {0x8, 4096},
    {0x9, 4608},
    {0xA, 5120},
    {0xB, 5632}, 
    {0xC, 6144},
    {0xD, 6656},
    {0xE, 7168},
    {0xF, 7680},
    {0x10, 8192}, 
};

struct eft_file {
    uint64_t magic; /* holds the magic number which so far always seems to be set to 1103806595072 */
    uint32_t height; /* holds the height code, with padding */
    uint32_t width; /* holds the width code, with padding */
    char* garbage; /* Tilemap data Emergency 4 uses. */
    unsigned char* data; /* S3TC/DXT1 texture data pointer */
};

struct eft_file input_file;

struct color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

unsigned char* rgba_stream;

/* Writes a single RGBA block from an EFT's tilemap into the RGBA buffer */
unsigned char* write_eft_tiles (unsigned char** input, int* tileindexes, int tilecount, int width, int height, int useBgra, bool swapWH) {

    /* create the output buffer */
    struct color* output = malloc(width * height * sizeof(struct color));
    if (output == NULL) return NULL;

    /* set blocknum to 0 then start copying tiles */
    int blocknum = 0;
    int height_stride = swapWH == true ? width : height;
	int width_stride = swapWH == true ? height : width;

    for (int y = 0; y < height_stride / 512; ++y) {
        for (int x = 0; x < width_stride / 512; ++x) {

            /* copy a single tile */
            for (int y_512 = 0; y_512 < 512; ++y_512) {
                for (int x_512 = 0; x_512 < 512; ++x_512) {
                    int x_offset_512 = (x_512 + 8) & 0x1FF;
                    int y_offset_512 = x_512 > 503 ? (y_512 + 4) & 0x1FF : y_512;

                    /* experimental tile data load (unused, some EFTs have strange tile data) */
                    int tile_address = blocknum;

                    /* This is where the copying actually happens. Use BGRA if desired. */
                    if (useBgra == 1) {
                        output [ ((y_512 + (y * 512)) * width_stride) + (x*512) + (x_512) ].b = input[tile_address][ (y_offset_512 * 4 * 512) + (4*x_offset_512) ]; //r
                        output [ ((y_512 + (y * 512)) * width_stride) + (x*512) + (x_512) ].g = input[tile_address][ (y_offset_512 * 4 * 512) + (4*x_offset_512) + 1]; //g
                        output [ ((y_512 + (y * 512)) * width_stride) + (x*512) + (x_512) ].r = input[tile_address][ (y_offset_512 * 4 * 512) + (4*x_offset_512) + 2]; //b
                        output [ ((y_512 + (y * 512)) * width_stride) + (x*512) + (x_512) ].a = input[tile_address][ (y_offset_512 * 4 * 512) + (4*x_offset_512) + 3]; //a
                    } else {
                        output [ ((y_512 + (y * 512)) * width_stride) + (x*512) + (x_512) ].r = input[tile_address][ (y_offset_512 * 4 * 512) + (4*x_offset_512) ]; //r
                        output [ ((y_512 + (y * 512)) * width_stride) + (x*512) + (x_512) ].g = input[tile_address][ (y_offset_512 * 4 * 512) + (4*x_offset_512) + 1]; //g
                        output [ ((y_512 + (y * 512)) * width_stride) + (x*512) + (x_512) ].b = input[tile_address][ (y_offset_512 * 4 * 512) + (4*x_offset_512) + 2]; //b
                        output [ ((y_512 + (y * 512)) * width_stride) + (x*512) + (x_512) ].a = input[tile_address][ (y_offset_512 * 4 * 512) + (4*x_offset_512) + 3]; //a
                    }
                }
            }
            blocknum+=1;
        }
    }

    return (char*)output;
}

/* converts an EFT tile's S3TC texture data to RGBA format. */
unsigned char* eft2rgba (unsigned char* input, int tileindex) {
    unsigned short color0; /* colour 0 */
    unsigned short color1; /* colour 1 */
    unsigned int codes; /* code stream to decode 4x4 block */

    /* allocate a 32-bit RGBA buffer. (Alpha channel is unused and will be set to 255) */
    struct color* rgba_buf = malloc(512 * 512 * 4 * sizeof(char));
    if (rgba_buf == NULL) {
        printf("Failed to allocate buffer\n");
        return NULL; /* failure to allocate buffer */
    }

    /* Calculate the 4x4 blockstep from the width and height. */
    int height_in_blocks = 512 / 4;
    int width_in_blocks = 512 / 4;

    for (int y = 0; y < height_in_blocks; y++) {
        for (int x = 0; x < width_in_blocks; x++) {
            /* first, copy the needed values from the heap. */
            memcpy(&color0, &input[ (131072*tileindex) + (y * 8 * width_in_blocks) + (x*8) ],        2);
            memcpy(&color1, &input[ (131072*tileindex) + (y * 8 * width_in_blocks) + (x*8) + 2],     2);
            memcpy(&codes,  &input[ (131072*tileindex) + (y * 8 * width_in_blocks) + (x*8) + 4 ],    4);

            /* next, reverse the bits (not negate, REVERSE) of the 32-bit integer so codes3, codes2, codes1 and codes0 are 
             * laid out in that order. 
             */
            unsigned int reversed_codes = codes; // r will be reversed bits of v; first get LSB of v
            unsigned char codes_u[4];
            codes_u[3] = (reversed_codes >> 24) & 0xFF;
            codes_u[2] = (reversed_codes >> 16) & 0xFF;
            codes_u[1] = (reversed_codes >> 8) & 0xFF;
            codes_u[0] = reversed_codes & 0xFF;
            reversed_codes = (codes_u[3] << 24) | (codes_u[2] << 16) | (codes_u[1] << 8) | codes_u[0];

            /* for each 4x4 block in the S3TC stream, read and reconstruct the RGB values of all 16 pixels from the block,
             * then write the block to the RGBA buffer.
             * 
             * RGB565 to RGB888 conversion borrowed from this Arduino forum post: 
             * https://forum.arduino.cc/t/help-converting-rgb565-to-rgb888/275681/2
             */
            unsigned char color0_rgb[3] = {  ( ( ( (color0 >> 11) & 0x1F ) * 527 ) + 23) >> 6,
                                    ( ( ( (color0 >> 5) & 0x3F ) * 259 ) + 33) >> 6, 
                                    ( ( ( color0 & 0x1F ) * 527 ) + 23) >> 6 };

            unsigned char color1_rgb[3] = {  ( ( ( (color1 >> 11) & 0x1F ) * 527 ) + 23) >> 6,
                                    ( ( ( (color1 >> 5) & 0x3F ) * 259 ) + 33) >> 6, 
                                    ( ( (  color1 & 0x1F ) * 527 ) + 23) >> 6 };

            /* now we scan through the 4x4 block, calculating pixel values then filling the RGBA buffer in the
             * right spots.
             * 
             * HERE BE DRAGONS: 
             * Array-addressing wise I literally have NO IDEA what I'm doing.
             */
            for (int yb = 0; yb < 4; ++yb) {
                for (int xb = 0; xb < 4; ++xb) {
                    /* get individual "pixels" from codes */
                    char rgb_row[3 * 4];

                    /* calculate the bishifts for unpacking the 4x4 block codes */
                    int bitshift_amount_x = xb * 2;
                    int bitshift_amount_y = yb * 4 * 2;
                    //printf("bitshift amount: %i\n", bitshift_amount_x+bitshift_amount_y);

                    if (color0 > color1) {
                        /* do the required conversions when color0 > color1 */
                        switch( (reversed_codes >> (bitshift_amount_x + bitshift_amount_y) ) & 0x3) {
                            case 0x0:
                                rgb_row[(xb*3)] = color0_rgb[0];
                                rgb_row[(xb*3)+1] = color0_rgb[1];
                                rgb_row[(xb*3)+2] = color0_rgb[2];
                                break;
                            case 0x1:
                                rgb_row[(xb*3)] = color1_rgb[0];
                                rgb_row[(xb*3)+1] = color1_rgb[1];
                                rgb_row[(xb*3)+2] = color1_rgb[2];
                                break;
                            case 0x2:
                                rgb_row[(xb*3)] = (2 * color0_rgb[0] + color1_rgb[0]) / 3;
                                rgb_row[(xb*3)+1] = (2 * color0_rgb[1] + color1_rgb[1]) / 3;
                                rgb_row[(xb*3)+2] = (2 * color0_rgb[2] + color1_rgb[2]) / 3;
                                break;
                            case 0x3:
                                rgb_row[(xb*3)] = (color0_rgb[0] + 2 * color1_rgb[0]) / 3;
                                rgb_row[(xb*3)+1] = (color0_rgb[1] + 2 * color1_rgb[1]) / 3;
                                rgb_row[(xb*3)+2] = (color0_rgb[2] + 2 * color1_rgb[2]) / 3;
                                break;
                        }
                    } 
                    else if (color0 <= color1) {
                        /* do the other conversion needed when color0 <= color1 */
                        switch( (reversed_codes >> (bitshift_amount_x + bitshift_amount_y) ) & 0x3) {
                            case 0x0:
                                rgb_row[(xb*3)] = color0_rgb[0];
                                rgb_row[(xb*3)+1] = color0_rgb[1];
                                rgb_row[(xb*3)+2] = color0_rgb[2];
                                break;
                            case 0x1:
                                rgb_row[(xb*3)] = color1_rgb[0];
                                rgb_row[(xb*3)+1] = color1_rgb[1];
                                rgb_row[(xb*3)+2] = color1_rgb[2];
                                break;
                            case 0x2:
                                rgb_row[(xb*3)] = ( color0_rgb[0] + color1_rgb[0] ) / 2;
                                rgb_row[(xb*3)+1] = ( color0_rgb[1] + color1_rgb[1] ) / 2;
                                rgb_row[(xb*3)+2] = ( color0_rgb[2] + color1_rgb[2] ) / 2;
                                break;
                            case 0x3:
                                rgb_row[(xb*3)] = 0;
                                rgb_row[(xb*3)+1] = 0;
                                rgb_row[(xb*3)+2] = 0;
                                break;
                        }
                    }
                    
                    /* set the RGBA values to the black colours computed */
                    rgba_buf[ ( (yb + y * 4) * 512) + (4 * x) + xb ].r = rgb_row[(3 * xb)];
                    rgba_buf[ ( (yb + y * 4) * 512) + (4 * x) + xb ].g = rgb_row[(3 * xb) + 1];
                    rgba_buf[ ( (yb + y * 4) * 512) + (4 * x) + xb ].b = rgb_row[(3 * xb) + 2];
                    rgba_buf[ ( (yb + y * 4) * 512) + (4 * x) + xb ].a = 255;
                }
            }
        }
    }

    /* return a pointer to our newly created buffer */
    return (char*)rgba_buf;
}

/* 
 * Loads an EFT file at filepath and returns a byte array
 * the byte array is then ready to be fed into any function
 * that uses RGBA as it's input format.
 */
extern __declspec(dllexport) unsigned char* load_eft_file_rgba (const char* filepath, int *width, int *height, bool swapWH) {
    /* first, check the file exists. */
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        return NULL;
    }

    /* scan the filesize and also populate the image data size */
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    size_t image_data_size = filesize - 1024; /* header information is 1024 bytes in size, mostly junk data lol */

    /* write the header */
    struct eft_file input_file;

    rewind(file);
    fread(&input_file.magic, 8, 1, file);
    char yes_or_no[4] = "no";
    if (input_file.magic != EFT_MAGIC_NUM) {
        printf("WARNING: EFT file seems to lack the magic number!\nThis file may not be an EFT\n");
        strncpy(yes_or_no, "no", 4);
    } else {
        strncpy(yes_or_no, "yes", 4);
    }
    int width_int, height_int;
    fread(&height_int, 4, 1, file);
    fread(&width_int, 4, 1, file);
	input_file.garbage = malloc(1012 * sizeof(char));
	if (input_file.garbage == NULL) {
        /* image malloc failed */
        fclose(file);
        return NULL;
    }

    fread(input_file.garbage, 1012, 1, file);

    /* fill the width and height based on the values in the header */
    for (int i = 0; i < 16; ++i) {
        if (width_int == image_size_table[i].code) {
            input_file.width = image_size_table[i].actualSize;
        }
        if (height_int == image_size_table[i].code) {
            input_file.height = image_size_table[i].actualSize;
        }
    }

    /* read the rest of the file data */
    input_file.data = malloc(filesize-0x400);
    if (input_file.data == NULL) {
        /* image malloc failed */
        fclose(file);
        return NULL;
    }
    rewind(file);
    fseek(file, 1024, SEEK_SET);
    fread(input_file.data, filesize-0x400, 1, file);
    fclose(file);

    unsigned char** tilemap = malloc(filesize-0x400);
    int tilecount = 0;
    if (tilemap == NULL) return NULL;
    else {
        for (int i = 0; i < (filesize-0x400) / 131072; ++i) {
            //populate all the tiles.
            tilemap[i] = eft2rgba(input_file.data, i);
            tilecount += 1;
        }
    }

    unsigned char* rgba_stream = write_eft_tiles(tilemap, (int*)input_file.garbage, tilecount, input_file.width, input_file.height, 0, swapWH);
    for (int i = 0; i < 256; ++i) {
		if (tilemap[i] != NULL) {
			free(tilemap[i]);
		}
	}
	if (tilemap != NULL) free(tilemap);
    if (input_file.garbage != NULL) free(input_file.garbage);
	if (input_file.data != NULL) free(input_file.data);
    if (rgba_stream == NULL) printf("oops! no image data... for some reason.\n");

    /* set the width and height arguments, then return the pointer */
    *width = swapWH == true ? input_file.height : input_file.width;
    *height = swapWH == true ? input_file.width : input_file.height;
    return rgba_stream;
}

/* 
 * Loads an EFT file at filepath and returns a byte array
 * the byte array is then ready to be fed into any function
 * that uses BGRA as it's input format.
 */
extern __declspec(dllexport) unsigned char* load_eft_file_bgra (const char* filepath, int *width, int *height, bool swapWH) {
    /* first, check the file exists. */
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        return NULL;
    }

    /* scan the filesize and also populate the image data size */
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    size_t image_data_size = filesize - 1024; /* header information is 1024 bytes in size, mostly junk data lol */

    /* write the header */
    struct eft_file input_file;

    rewind(file);
    fread(&input_file.magic, 8, 1, file);
    char yes_or_no[4] = "no";
    if (input_file.magic != EFT_MAGIC_NUM) {
        printf("WARNING: EFT file seems to lack the magic number!\nThis file may not be an EFT\n");
        strncpy(yes_or_no, "no", 4);
    } else {
        strncpy(yes_or_no, "yes", 4);
    }
    int width_int, height_int;
    fread(&height_int, 4, 1, file);
    fread(&width_int, 4, 1, file);
	input_file.garbage = malloc(1012 * sizeof(char));
	if (input_file.garbage == NULL) {
		fclose(file);
		return NULL;
	}
    fread(input_file.garbage, 1012, 1, file);

    /* fill the width and height based on the values in the header */
    for (int i = 0; i < 16; ++i) {
        if (width_int == image_size_table[i].code) {
            input_file.width = image_size_table[i].actualSize;
        }
        if (height_int == image_size_table[i].code) {
            input_file.height = image_size_table[i].actualSize;
        }
    }

    /* read the rest of the file data */
    input_file.data = malloc(filesize-0x400);
    if (input_file.data == NULL) {
        /* image malloc failed */
        fclose(file);
        return NULL;
    }
    rewind(file);
    fseek(file, 1024, SEEK_SET);
    fread(input_file.data, filesize-0x400, 1, file);
    fclose(file);

	unsigned char** tilemap = malloc(filesize-0x400);
    int tilecount = 0;
    if (tilemap == NULL) return NULL;
    else {
        for (int i = 0; i < (filesize-0x400) / 131072; ++i) {
            //populate all the tiles.
            tilemap[i] = eft2rgba(input_file.data, i);
            tilecount += 1;
        }
    }

    unsigned char* rgba_stream = write_eft_tiles(tilemap, (int*)input_file.garbage, tilecount, input_file.width, input_file.height, 1, swapWH);
    for (int i = 0; i < 256; ++i) {
		if (tilemap[i] != NULL) {
			free(tilemap[i]);
		}
	}
    if (tilemap != NULL) free(tilemap);
    if (input_file.garbage != NULL) free(input_file.garbage);
	if (input_file.data != NULL) free(input_file.data);
    if (rgba_stream == NULL) printf("oops! no image data... for some reason.\n");

    /* set the width and height arguments, then return the pointer */
    *width = swapWH == true ? input_file.height : input_file.width;
    *height = swapWH == true ? input_file.width : input_file.height;
    return rgba_stream;
}

extern __declspec(dllexport) void free_eft_memory (unsigned char* eft_ptr) {
	if (eft_ptr != NULL) {
		free(eft_ptr);
		eft_ptr = NULL;
	} else {
		printf("eft_loader: WARNING: attempted to free a null pointer!\n");
	}
}

extern unsigned char* load_eft_file_s3tc (const char* filepath, int *width, int *height) {
    /* first, check the file exists. */
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        return NULL;
    }

    /* scan the filesize and also populate the image data size */
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    size_t image_data_size = filesize - 1024; /* header information is 1024 bytes in size, mostly junk data lol */

    /* write the header */
    struct eft_file input_file;

    rewind(file);
    fread(&input_file.magic, 8, 1, file);
    char yes_or_no[4] = "no";
    if (input_file.magic != EFT_MAGIC_NUM) {
        printf("WARNING: EFT file seems to lack the magic number!\nThis file may not be an EFT\n");
        strncpy(yes_or_no, "no", 4);
    } else {
        strncpy(yes_or_no, "yes", 4);
    }
    int width_int, height_int;
    fread(&height_int, 4, 1, file);
    fread(&width_int, 4, 1, file);

    input_file.garbage = malloc(1012 * sizeof(char));
    if (input_file.garbage == NULL) return NULL;
    fread(input_file.garbage, 1012, 1, file);

    /* fill the width and height based on the values in the header */
    for (int i = 0; i < 16; ++i) {
        if (width_int == image_size_table[i].code) {
            input_file.width = image_size_table[i].actualSize;
        }
        if (height_int == image_size_table[i].code) {
            input_file.height = image_size_table[i].actualSize;
        }
    }

    /* read the rest of the file data */
    unsigned char* input_file_data = malloc(filesize - 0x410);
    if (input_file_data == NULL) {
        /* image malloc failed */
        fclose(file);
        return NULL;
    }
    rewind(file);
    fseek(file, 1024+131072, SEEK_SET);
    fread(input_file_data, filesize - 0x410, 1, file);
    fclose(file);

    /* now we have the width, height and image data, let's load the texture stream */
    return input_file_data;
}

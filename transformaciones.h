#include <stdio.h>
#include <stdlib.h>

// inversion horizontal a color
extern void inv_hor_color(char nombre_imagen[80]) {
    FILE *image = fopen(nombre_imagen, "rb");
    FILE *outputImage = fopen("imagen_horizontal_color.bmp", "wb");

    if (image == NULL || outputImage == NULL) {
        printf("Error al abrir los archivos.\n");
        return;
    }

    long ancho, alto;
    unsigned char r, g, b;
    unsigned char xx[54];

    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    ancho = *(int*)&xx[18];
    alto  = *(int*)&xx[22];
    if (alto < 0) alto = -alto;

    int padding = (4 - (ancho * 3) % 4) % 4;

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));

    int j = 0;
    while (!feof(image) && j < (ancho * alto)) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;

        if ((j + 1) % ancho == 0) {
            for (int p = 0; p < padding; p++) fgetc(image);
        }
        j++;
    }

    for (int i = 0; i < ancho * alto; i++) {
        int fila = i / ancho;
        int col = i % ancho;
        int idx = fila * ancho + (ancho - 1 - col);

        fputc(arr_in_b[idx], outputImage);
        fputc(arr_in_g[idx], outputImage);
        fputc(arr_in_r[idx], outputImage);

        if ((i + 1) % ancho == 0) {
            for (int p = 0; p < padding; p++) fputc(0, outputImage);
        }
    }

    free(arr_in_b); 
    free(arr_in_g); 
    free(arr_in_r);
    fclose(image); 
    fclose(outputImage);
}

// inversion horizontal en blanco y negro
extern void inv_hor_bn(char nombre_imagen[80]) {
    FILE *image = fopen(nombre_imagen, "rb");
    FILE *outputImage = fopen("imagen_horizontal_bn.bmp", "wb");

    if (image == NULL || outputImage == NULL) {
        printf("Error al abrir los archivos.\n");
        return;
    }

    long ancho, alto;
    unsigned char r, g, b;
    unsigned char xx[54];

    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    ancho = *(int*)&xx[18];
    alto  = *(int*)&xx[22];
    if (alto < 0) alto = -alto;

    int padding = (4 - (ancho * 3) % 4) % 4;

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));

    int j = 0;
    while (!feof(image) && j < (ancho * alto)) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;

        if ((j + 1) % ancho == 0) {
            for (int p = 0; p < padding; p++) fgetc(image);
        }
        j++;
    }

    for (int i = 0; i < ancho * alto; i++) {
        int fila = i / ancho;
        int col = i % ancho;
        int idx = fila * ancho + (ancho - 1 - col);

        unsigned char pixel = 0.21 * arr_in_r[idx] + 0.72 * arr_in_g[idx] + 0.07 * arr_in_b[idx];

        fputc(pixel, outputImage);
        fputc(pixel, outputImage);
        fputc(pixel, outputImage);

        if ((i + 1) % ancho == 0) {
            for (int p = 0; p < padding; p++) fputc(0, outputImage);
        }
    }

    free(arr_in_b); 
    free(arr_in_g); 
    free(arr_in_r);
    fclose(image); 
    fclose(outputImage);
}

// inversion vertical a color
extern void inv_ver_color(char nombre_imagen[80]) {
    FILE *image = fopen(nombre_imagen, "rb");
    FILE *outputImage = fopen("imagen_vertical_color.bmp", "wb");

    if (image == NULL || outputImage == NULL) {
        printf("Error al abrir los archivos.\n");
        return;
    }

    long ancho, alto;
    unsigned char r, g, b;
    unsigned char xx[54];

    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    ancho = *(int*)&xx[18];
    alto  = *(int*)&xx[22];
    if (alto < 0) alto = -alto;

    int padding = (4 - (ancho * 3) % 4) % 4;

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));

    int j = 0;
    while (!feof(image) && j < (ancho * alto)) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;

        if ((j + 1) % ancho == 0) {
            for (int p = 0; p < padding; p++) fgetc(image);
        }
        j++;
    }

    for (int i = 0; i < ancho * alto; i++) {
        int fila = i / ancho;
        int col = i % ancho;
        int idx = (alto - 1 - fila) * ancho + col;

        fputc(arr_in_b[idx], outputImage);
        fputc(arr_in_g[idx], outputImage);
        fputc(arr_in_r[idx], outputImage);

        if ((i + 1) % ancho == 0) {
            for (int p = 0; p < padding; p++) fputc(0, outputImage);
        }
    }

    free(arr_in_b); 
    free(arr_in_g); 
    free(arr_in_r);
    fclose(image); 
    fclose(outputImage);
}

// inversion vertical en blanco y negro
extern void inv_ver_bn(char nombre_imagen[80]) {
    FILE *image = fopen(nombre_imagen, "rb");
    FILE *outputImage = fopen("imagen_vertical_bn.bmp", "wb");

    if (image == NULL || outputImage == NULL) {
        printf("Error al abrir los archivos.\n");
        return;
    }

    long ancho, alto;
    unsigned char r, g, b;
    unsigned char xx[54];

    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);
    }

    ancho = *(int*)&xx[18];
    alto  = *(int*)&xx[22];
    if (alto < 0) alto = -alto;

    int padding = (4 - (ancho * 3) % 4) % 4;

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));

    int j = 0;
    while (!feof(image) && j < (ancho * alto)) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;

        if ((j + 1) % ancho == 0) {
            for (int p = 0; p < padding; p++) fgetc(image);
        }
        j++;
    }

    for (int i = 0; i < ancho * alto; i++) {
        int fila = i / ancho;
        int col = i % ancho;
        int idx = (alto - 1 - fila) * ancho + col;

        unsigned char pixel = 0.21 * arr_in_r[idx] + 0.72 * arr_in_g[idx] + 0.07 * arr_in_b[idx];

        fputc(pixel, outputImage);
        fputc(pixel, outputImage);
        fputc(pixel, outputImage);

        if ((i + 1) % ancho == 0) {
            for (int p = 0; p < padding; p++) fputc(0, outputImage);
        }
    }

    free(arr_in_b); 
    free(arr_in_g); 
    free(arr_in_r);
    fclose(image); 
    fclose(outputImage);
}

// desenfoque a color
extern void blur_color(char nombre_imagen[80], int kernel_size) {
    FILE *image = fopen(nombre_imagen, "rb");
    FILE *outputImage = fopen("imagen_blur_color.bmp", "wb");

    if (!image || !outputImage) {
        printf("Error al abrir los archivos.\n");
        return;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, image);
    fwrite(header, sizeof(unsigned char), 54, outputImage);

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    if (height < 0) height = -height;

    int row_padded = (width * 3 + 3) & (~3);

    unsigned char** input_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));
    unsigned char** output_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));
    unsigned char** temp_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));

    for (int i = 0; i < height; i++) {
        input_rows[i] = (unsigned char*)malloc(row_padded);
        output_rows[i] = (unsigned char*)malloc(row_padded);
        temp_rows[i] = (unsigned char*)malloc(row_padded);
        fread(input_rows[i], sizeof(unsigned char), row_padded, image);
    }

    int k = kernel_size / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;

            for (int dx = -k; dx <= k; dx++) {
                int nx = x + dx;
                if (nx >= 0 && nx < width) {
                    int idx = nx * 3;
                    sumB += input_rows[y][idx + 0];
                    sumG += input_rows[y][idx + 1];
                    sumR += input_rows[y][idx + 2];
                    count++;
                }
            }

            int index = x * 3;
            temp_rows[y][index + 0] = sumB / count;
            temp_rows[y][index + 1] = sumG / count;
            temp_rows[y][index + 2] = sumR / count;
        }

        for (int p = width * 3; p < row_padded; p++) {
            temp_rows[y][p] = input_rows[y][p];
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;

            for (int dy = -k; dy <= k; dy++) {
                int ny = y + dy;
                if (ny >= 0 && ny < height) {
                    int idx = x * 3;
                    sumB += temp_rows[ny][idx + 0];
                    sumG += temp_rows[ny][idx + 1];
                    sumR += temp_rows[ny][idx + 2];
                    count++;
                }
            }

            int index = x * 3;
            output_rows[y][index + 0] = sumB / count;
            output_rows[y][index + 1] = sumG / count;
            output_rows[y][index + 2] = sumR / count;
        }

        for (int p = width * 3; p < row_padded; p++) {
            output_rows[y][p] = temp_rows[y][p];
        }
    }

    for (int i = 0; i < height; i++) {
        fwrite(output_rows[i], sizeof(unsigned char), row_padded, outputImage);
        free(input_rows[i]);
        free(temp_rows[i]);
        free(output_rows[i]);
    }

    free(input_rows);
    free(temp_rows);
    free(output_rows);
    fclose(image);
    fclose(outputImage);
}

// desenfoque en blanco y negro
extern void blur_bn(char nombre_imagen[80], int kernel_size) {
    FILE *image = fopen(nombre_imagen, "rb");
    FILE *outputImage = fopen("imagen_blur_bn.bmp", "wb");

    if (!image || !outputImage) {
        printf("Error al abrir los archivos.\n");
        return;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, image);
    fwrite(header, sizeof(unsigned char), 54, outputImage);

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    if (height < 0) height = -height;

    int row_padded = (width * 3 + 3) & (~3);

    unsigned char** input_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));
    unsigned char** output_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));
    unsigned char** temp_rows = (unsigned char**)malloc(height * sizeof(unsigned char*));

    for (int i = 0; i < height; i++) {
        input_rows[i] = (unsigned char*)malloc(row_padded);
        output_rows[i] = (unsigned char*)malloc(row_padded);
        temp_rows[i] = (unsigned char*)malloc(row_padded);
        fread(input_rows[i], sizeof(unsigned char), row_padded, image);
    }

    int k = kernel_size / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;

            for (int dx = -k; dx <= k; dx++) {
                int nx = x + dx;
                if (nx >= 0 && nx < width) {
                    int idx = nx * 3;
                    sumB += input_rows[y][idx + 0];
                    sumG += input_rows[y][idx + 1];
                    sumR += input_rows[y][idx + 2];
                    count++;
                }
            }

            int index = x * 3;
            temp_rows[y][index + 0] = sumB / count;
            temp_rows[y][index + 1] = sumG / count;
            temp_rows[y][index + 2] = sumR / count;
        }

        for (int p = width * 3; p < row_padded; p++) {
            temp_rows[y][p] = input_rows[y][p];
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sumB = 0, sumG = 0, sumR = 0, count = 0;

            for (int dy = -k; dy <= k; dy++) {
                int ny = y + dy;
                if (ny >= 0 && ny < height) {
                    int idx = x * 3;
                    sumB += temp_rows[ny][idx + 0];
                    sumG += temp_rows[ny][idx + 1];
                    sumR += temp_rows[ny][idx + 2];
                    count++;
                }
            }

            int index = x * 3;
            unsigned char avgB = sumB / count;
            unsigned char avgG = sumG / count;
            unsigned char avgR = sumR / count;

            unsigned char pixel = 0.21 * avgR + 0.72 * avgG + 0.07 * avgB;

            output_rows[y][index + 0] = pixel;
            output_rows[y][index + 1] = pixel;
            output_rows[y][index + 2] = pixel;
        }

        for (int p = width * 3; p < row_padded; p++) {
            output_rows[y][p] = temp_rows[y][p];
        }
    }

    for (int i = 0; i < height; i++) {
        fwrite(output_rows[i], sizeof(unsigned char), row_padded, outputImage);
        free(input_rows[i]);    
        free(temp_rows[i]);
        free(output_rows[i]);
    }

    free(input_rows);
    free(temp_rows);
    free(output_rows);
    fclose(image);
    fclose(outputImage);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




extern void gray_img(char *nombre){

    FILE *image, *outputImage, *lecturas;
    image = fopen(nombre,"rb"); //Imagen original a transformar
    outputImage = fopen("./img/gray_img.bmp","wb"); //Imagen transformada
    unsigned char r, g, b; //Pixel
    for(int i=0; i<54; i++) fputc(fgetc(image), outputImage); //Copia cabecera a nueva imagen

        while(!feof(image)){ //Grises
            b = fgetc(image);
            g = fgetc(image);
            r = fgetc(image);
            unsigned char pixel = 0.21*r+0.72*g+0.07*b;
            fputc(pixel, outputImage);
            fputc(pixel, outputImage);
            fputc(pixel, outputImage);
        }
        fclose(image);
    fclose(outputImage);
    return;
}


extern void inv_ver_color(char mask[10], char path[80]) {
    FILE *image, *outputImage, *lecturas, *fptr;

    // char PATH_0[120] = "/Users/emmanueltorres/Documents/image_proces_c/c_openmp/img/";

    char add_char[80] = "./img/";
    strcat(add_char, mask);
    strcat(add_char, ".bmp");

    printf("%s\n", add_char);

    image = fopen(path, "rb"); // Original Image
    outputImage = fopen(add_char, "wb");

    // Definition of variables
    int i, j, tam1;
    long ancho, tam, bpp;
    long alto;

    unsigned char r, g, b, pixel; // Pixel
    unsigned char xx[54];

    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage); // Copia cabecera a nueva imagen
    }

    tam = (long)xx[4] * 65536 + (long)xx[3] * 256 + (long)xx[2];
    bpp = (long)xx[29] * 256 + (long)xx[28];
    ancho = (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
    alto = (long)xx[24] * 65536 + (long)xx[23] * 256 + (long)xx[22];

    tam1 = tam;

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));

    j = 0;

    while (!feof(image)) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;

        j++;
    }

    // Escribir píxeles invertidos (vertical: filas de abajo hacia arriba)
    for (int i = alto - 1; i >= 0; i--) {
        for (int y = 0; y < ancho; y++) {
            int index = (i * ancho) + y;

            fputc(arr_in_b[index], outputImage);
            fputc(arr_in_g[index], outputImage);
            fputc(arr_in_r[index], outputImage);
        }
    }

    // Escritura en archivo de registro
    FILE *outputLog = fopen("output_log.txt", "a");

    if (outputLog == NULL) {
        fprintf(stderr, "Error: No se pudo crear o abrir el archivo de registro.\n");

        fclose(image);
        fclose(outputImage);

        free(arr_in_b);
        free(arr_in_g);
        free(arr_in_r);

        return;
    }

    printf("inversion de imagen vertical a colores\n");

    fprintf(outputLog, "Función: inv_img_color_vertical, con %s\n", path);
    fprintf(outputLog, "Localidades totales leídas: %ld\n", ancho * alto);
    fprintf(outputLog, "Localidades totales escritas: %ld\n", ancho * alto);
    fprintf(outputLog, "-------------------------------------\n");

    fclose(outputLog);

    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);

    fclose(image);
    fclose(outputImage);
}



extern void blur(const char* input_path, const char* name_output, int kernel_size) {
    FILE *image, *outputImage;
    char output_path[100] = "./img/";

    strcat(output_path, name_output);
    strcat(output_path, ".bmp");

    printf("%s\n", output_path);

    image = fopen(input_path, "rb");
    outputImage = fopen(output_path, "wb");

    if (!image || !outputImage) {
        printf("Error abriendo archivos.\n");
        return;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, image);
    fwrite(header, sizeof(unsigned char), 54, outputImage);

    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
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

    // Paso intermedio: desenfoque horizontal
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

        // Copiar padding sin modificar
        for (int p = width * 3; p < row_padded; p++) {
            temp_rows[y][p] = input_rows[y][p];
        }
    }

    // Paso final: desenfoque vertical
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

        // Copiar padding sin modificar
        for (int p = width * 3; p < row_padded; p++) {
            output_rows[y][p] = temp_rows[y][p];
        }
    }

    // Escritura final
    for (int i = 0; i < height; i++) {
        fwrite(output_rows[i], sizeof(unsigned char), row_padded, outputImage);

        free(input_rows[i]);
        free(temp_rows[i]);
        free(output_rows[i]);
    }

    // Escritura en archivo de registro
    FILE *outputLog = fopen("output_log.txt", "a");

    if (outputLog == NULL) {
        fprintf(stderr, "Error: No se pudo crear o abrir el archivo de registro.\n");

        fclose(image);
        fclose(outputImage);

        free(input_rows);
        free(temp_rows);
        free(output_rows);

        return;
    }

    printf("Desenfoque de imagen a escala de grises\n");

    fprintf(outputLog, "Función: desenfoque, con %s\n", input_path);
    fprintf(outputLog, "Localidades totales leídas: %d\n", width * height);
    fprintf(outputLog, "Localidades totales escritas: %d\n", width * height);
    fprintf(outputLog, "-------------------------------------\n");

    fclose(outputLog);

    free(input_rows);
    free(temp_rows);
    free(output_rows);

    fclose(image);
    fclose(outputImage);
}



extern void inv_hor_color(char mask[10], char path[80]) {
    FILE *image, *outputImage, *lecturas, *fptr;

    // char PATH_0[120] = "/Users/emmanueltorres/Documents/image_proces_c/c_openmp/img/";

    char add_char[80] = "./img/";
    strcat(add_char, mask);
    strcat(add_char, ".bmp");

    printf("%s\n", add_char);

    image = fopen(path, "rb"); // Original Image
    outputImage = fopen(add_char, "wb");

    // Definition of variables
    int i, j, tam1;
    long ancho, tam, bpp;
    long alto;

    unsigned char r, g, b, pixel; // Pixel
    unsigned char xx[54];

    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage); // Copia cabecera a nueva imagen
    }

    tam = (long)xx[4] * 65536 + (long)xx[3] * 256 + (long)xx[2];
    bpp = (long)xx[29] * 256 + (long)xx[28];
    ancho = (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
    alto = (long)xx[24] * 65536 + (long)xx[23] * 256 + (long)xx[22];

    tam1 = tam;

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho * alto * sizeof(unsigned char));

    j = 0;

    while (!feof(image)) {
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);

        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;

        j++;
    }

    // Escribir píxeles invertidos
    for (int i = 0; i < alto; i++) {
        for (int y = ancho; y > 0; y--) {
            int index = (i * ancho) + y;

            fputc(arr_in_b[index], outputImage);
            fputc(arr_in_g[index], outputImage);
            fputc(arr_in_r[index], outputImage);
        }
    }

    // Escritura en archivo de registro
    FILE *outputLog = fopen("output_log.txt", "a");

    if (outputLog == NULL) {
        fprintf(stderr, "Error: No se pudo crear o abrir el archivo de registro.\n");

        fclose(image);
        fclose(outputImage);

        free(arr_in_b);
        free(arr_in_g);
        free(arr_in_r);

        return;
    }

    printf("inversion de imagen horizontal a colores\n");

    fprintf(outputLog, "Función: inv_img_color_horizontal, con %s\n", path);
    fprintf(outputLog, "Localidades totales leídas: %ld\n", ancho * alto);
    fprintf(outputLog, "Localidades totales escritas: %ld\n", ancho * alto);
    fprintf(outputLog, "-------------------------------------\n");

    fclose(outputLog);

    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);

    fclose(image);
    fclose(outputImage);
}

// Deteccion de bordes por derivada parcial en X (Sobel X)
extern void edge_x(char nombre_imagen[80]) {
    FILE *image = fopen(nombre_imagen, "rb");
    FILE *outputImage = fopen("imagen_edge_x.bmp", "wb");

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

    for (int i = 0; i < height; i++) {
        input_rows[i] = (unsigned char*)malloc(row_padded);
        output_rows[i] = (unsigned char*)malloc(row_padded);
        fread(input_rows[i], sizeof(unsigned char), row_padded, image);
    }

    // Kernel Sobel X: [-1, 0, 1 / -2, 0, 2 / -1, 0, 1]
    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sumX = 0;

            for (int dy = -1; dy <= 1; dy++) {
                int ny = y + dy;
                if (ny >= 0 && ny < height) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = x + dx;
                        if (nx >= 0 && nx < width) {
                            int idx = nx * 3;
                            unsigned char b = input_rows[ny][idx + 0];
                            unsigned char g = input_rows[ny][idx + 1];
                            unsigned char r = input_rows[ny][idx + 2];
                            unsigned char gray = 0.21 * r + 0.72 * g + 0.07 * b;

                            sumX += gray * Gx[dy + 1][dx + 1];
                        }
                    }
                }
            }

            int val = abs(sumX);
            if (val > 255) val = 255;
            unsigned char pixel = (unsigned char)val;

            int index = x * 3;
            output_rows[y][index + 0] = pixel;
            output_rows[y][index + 1] = pixel;
            output_rows[y][index + 2] = pixel;
        }

        for (int p = width * 3; p < row_padded; p++) {
            output_rows[y][p] = input_rows[y][p];
        }
    }

    for (int i = 0; i < height; i++) {
        fwrite(output_rows[i], sizeof(unsigned char), row_padded, outputImage);
        free(input_rows[i]);
        free(output_rows[i]);
    }

    free(input_rows);
    free(output_rows);
    fclose(image);
    fclose(outputImage);
}

// Deteccion de bordes por derivada parcial en Y (Sobel Y)
extern void edge_y(char nombre_imagen[80]) {
    FILE *image = fopen(nombre_imagen, "rb");
    FILE *outputImage = fopen("imagen_edge_y.bmp", "wb");

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

    for (int i = 0; i < height; i++) {
        input_rows[i] = (unsigned char*)malloc(row_padded);
        output_rows[i] = (unsigned char*)malloc(row_padded);
        fread(input_rows[i], sizeof(unsigned char), row_padded, image);
    }

    // Kernel Sobel Y: [-1, -2, -1 / 0, 0, 0 / 1, 2, 1]
    int Gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sumY = 0;

            for (int dy = -1; dy <= 1; dy++) {
                int ny = y + dy;
                if (ny >= 0 && ny < height) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = x + dx;
                        if (nx >= 0 && nx < width) {
                            int idx = nx * 3;
                            unsigned char b = input_rows[ny][idx + 0];
                            unsigned char g = input_rows[ny][idx + 1];
                            unsigned char r = input_rows[ny][idx + 2];
                            unsigned char gray = 0.21 * r + 0.72 * g + 0.07 * b;

                            sumY += gray * Gy[dy + 1][dx + 1];
                        }
                    }
                }
            }

            int val = abs(sumY);
            if (val > 255) val = 255;
            unsigned char pixel = (unsigned char)val;

            int index = x * 3;
            output_rows[y][index + 0] = pixel;
            output_rows[y][index + 1] = pixel;
            output_rows[y][index + 2] = pixel;
        }

        for (int p = width * 3; p < row_padded; p++) {
            output_rows[y][p] = input_rows[y][p];
        }
    }

    for (int i = 0; i < height; i++) {
        fwrite(output_rows[i], sizeof(unsigned char), row_padded, outputImage);
        free(input_rows[i]);
        free(output_rows[i]);
    }

    free(input_rows);
    free(output_rows);
    fclose(image);
    fclose(outputImage);
}



extern void inv_hor_gray(char mask[10], char path[80]) {
    inv_hor_color(mask, path);
}

extern void inv_ver_gray(char mask[10], char path[80]) {
    inv_ver_color(mask, path);
}

extern void blur_gray(const char* input_path, const char* name_output, int kernel_size) {
    blur(input_path, name_output, kernel_size);
}

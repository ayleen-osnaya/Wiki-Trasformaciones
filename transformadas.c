#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "elemento.h"

#define NUM_THREADS 8
#define TOTAL_IMAGENES 10
#define KERNEL_SIZE 5

// Comprueba que la imagen de entrada exista antes de llamar a las funciones del .h.
static int archivo_existe(const char *ruta)
{
    FILE *archivo = fopen(ruta, "rb");
    if (archivo == NULL) {
        return 0;
    }
    fclose(archivo);
    return 1;
}

// Copia un archivo binario. Regresa 0 si la copia fue correcta.
static int copiar_archivo(const char *origen, const char *destino)
{
    FILE *entrada = fopen(origen, "rb");
    FILE *salida;
    unsigned char buffer[8192];
    size_t leidos;
    int error = 0;
    if (entrada == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo de origen: %s\n", origen);
        return -1;
    }
    salida = fopen(destino, "wb");
    if (salida == NULL) {
        fprintf(stderr, "No se pudo crear el archivo de destino: %s\n", destino);
        fclose(entrada);
        return -1;
    }
    while ((leidos = fread(buffer, 1, sizeof(buffer), entrada)) > 0) {
        if (fwrite(buffer, 1, leidos, salida) != leidos) {
            error = 1;
            break;
        }
    }
    if (ferror(entrada)) {
        error = 1;
    }
    fclose(entrada);
    if (fclose(salida) != 0) {
        error = 1;
    }
    if (error) {
        fprintf(stderr, "Error al copiar %s hacia %s\n", origen, destino);
        remove(destino);
        return -1;
    }
    return 0;
}

// Mueve un archivo copiandolo al destino y eliminando el archivo original.
static void mover_archivo(const char *origen, const char *destino)
{
    if (copiar_archivo(origen, destino) == 0) {
        if (remove(origen) != 0) {
            fprintf(stderr, "No se pudo eliminar el archivo temporal: %s\n", origen);
        }
    }
}

// gray_img() siempre genera ./img/gray_img.bmp; se copia con un nombre unico
// a la carpeta de la imagen que se esta procesando.
static int generar_gris_por_imagen(const char *input_path, const char *gris_destino)
{
    gray_img((char *)input_path);
    return copiar_archivo(".\\img\\gray_img.bmp", gris_destino);
}

// Las funciones de inversion y blur reciben un nombre y generan ./img/<nombre>.bmp.
static void mover_desde_img(const char *mascara,
                            const char *nombre_final,
                            const char *directorio_imagen)
{
    char origen[160];
    char destino[256];

    snprintf(origen, sizeof(origen), ".\\img\\%s.bmp", mascara);
    snprintf(destino, sizeof(destino), "%s\\%s", directorio_imagen, nombre_final);

    mover_archivo(origen, destino);
}

int main(int argc, char *argv[])
{
    char *imagenes[TOTAL_IMAGENES] = {
        ".\\imagenes\\Imagen1.bmp",
        ".\\imagenes\\Imagen2.bmp",
        ".\\imagenes\\Imagen3.bmp",
        ".\\imagenes\\Imagen4.bmp",
        ".\\imagenes\\Imagen5.bmp",
        ".\\imagenes\\Imagen6.bmp",
        ".\\imagenes\\Imagen7.bmp",
        ".\\imagenes\\Imagen8.bmp",
        ".\\imagenes\\Imagen9.bmp",
        ".\\imagenes\\Imagen10.bmp"
    };

    int hilos_usados = 0;
    int imagenes_procesadas = 0;
    double inicio, fin;
    int kernel_gris = KERNEL_SIZE;
    int kernel_color = KERNEL_SIZE;

    // Kernel variable desde la interfaz: --kernel-gray N  --kernel-color N
    for (int i = 1; i + 1 < argc; i++) {
        if (strcmp(argv[i], "--kernel-gray") == 0) {
            kernel_gris = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--kernel-color") == 0) {
            kernel_color = atoi(argv[++i]);
        }
    }
    if (kernel_gris < 1) kernel_gris = KERNEL_SIZE;
    if (kernel_color < 1) kernel_color = KERNEL_SIZE;

    // Desactivar el ajuste dinamico para solicitar el equipo de hilos indicado.
    omp_set_dynamic(0);
    omp_set_num_threads(NUM_THREADS);

    // Eliminar resultados de ejecuciones anteriores y preparar directorios.
    system("if exist \"resultados\" rmdir /s /q \"resultados\"");
    system("mkdir \"resultados\" >nul 2>nul");
    system("if not exist \"img\" mkdir \"img\"");
    system("del /q \"imagen_edge_x.bmp\" \"imagen_edge_y.bmp\" 2>nul");

    inicio = omp_get_wtime();

    printf("Procesando %d imagenes con %d hilos...\n\n",
           TOTAL_IMAGENES, NUM_THREADS);

    for (int k = 0; k < TOTAL_IMAGENES; k++) {
        int idx_img = k + 1;
        char dir_img[128];
        char ruta_gris[256];
        char comando_mkdir[256];

        // Nombres unicos: cada transformacion escribe un archivo diferente en ./img/.
        char mask_ihc[16], mask_ihg[16];
        char mask_ivc[16], mask_ivg[16];
        char out_blur_c[16], out_blur_g[16];

        printf("[%d/%d] Procesando %s\n", idx_img, TOTAL_IMAGENES, imagenes[k]);

        if (!archivo_existe(imagenes[k])) {
            fprintf(stderr, "No existe la imagen: %s. Se omite.\n\n", imagenes[k]);
            continue;
        }

        // Crear la carpeta de salida correspondiente a esta imagen.
        snprintf(dir_img, sizeof(dir_img), "resultados\\Imagen%d", idx_img);
        snprintf(comando_mkdir, sizeof(comando_mkdir),
                 "mkdir \"%s\" >nul 2>nul", dir_img);

        if (system(comando_mkdir) != 0) {
            fprintf(stderr, "No se pudo crear el directorio %s\n\n", dir_img);
            continue;
        }

        snprintf(ruta_gris, sizeof(ruta_gris), "%s\\base_gray.bmp", dir_img);

        // Crear la imagen base en escala de grises antes de iniciar los hilos.
        if (generar_gris_por_imagen(imagenes[k], ruta_gris) != 0) {
            fprintf(stderr, "No se pudo generar la imagen en grises para %s\n\n",
                    imagenes[k]);
            continue;
        }

        snprintf(mask_ihc, sizeof(mask_ihc), "ihc_%d", idx_img);
        snprintf(mask_ihg, sizeof(mask_ihg), "ihg_%d", idx_img);
        snprintf(mask_ivc, sizeof(mask_ivc), "ivc_%d", idx_img);
        snprintf(mask_ivg, sizeof(mask_ivg), "ivg_%d", idx_img);
        snprintf(out_blur_c, sizeof(out_blur_c), "bc_%d", idx_img);
        snprintf(out_blur_g, sizeof(out_blur_g), "bg_%d", idx_img);

        // Ocho secciones: cada una ejecuta una transformacion independiente.
        // La region paralela termina antes de mover los archivos de salida.
        #pragma omp parallel num_threads(NUM_THREADS) shared(hilos_usados)
        {
            #pragma omp single
            {
                hilos_usados = omp_get_num_threads();
            }

            #pragma omp sections
            {
                #pragma omp section
                {
                    inv_hor_color(mask_ihc, imagenes[k]);
                }

                #pragma omp section
                {
                    inv_hor_gray(mask_ihg, ruta_gris);
                }

                #pragma omp section
                {
                    inv_ver_color(mask_ivc, imagenes[k]);
                }

                #pragma omp section
                {
                    inv_ver_gray(mask_ivg, ruta_gris);
                }

                #pragma omp section
                {
                    blur(imagenes[k], out_blur_c, kernel_color);
                }

                #pragma omp section
                {
                    blur_gray(ruta_gris, out_blur_g, kernel_gris);
                }

                #pragma omp section
                {
                    edge_x(imagenes[k]);
                }

                #pragma omp section
                {
                    edge_y(imagenes[k]);
                }
            }
        }

        // Mover las seis transformaciones que se guardaron dentro de ./img/.
        mover_desde_img(mask_ihc, "inv_hor_color.bmp", dir_img);
        mover_desde_img(mask_ihg, "inv_hor_bn.bmp", dir_img);
        mover_desde_img(mask_ivc, "inv_ver_color.bmp", dir_img);
        mover_desde_img(mask_ivg, "inv_ver_bn.bmp", dir_img);
        mover_desde_img(out_blur_c, "blur_color.bmp", dir_img);
        mover_desde_img(out_blur_g, "blur_bn.bmp", dir_img);

        // edge_x() y edge_y() escriben en el directorio actual, no en ./img/.
        {
            char destino[256];

            snprintf(destino, sizeof(destino), "%s\\edge_x.bmp", dir_img);
            mover_archivo("imagen_edge_x.bmp", destino);

            snprintf(destino, sizeof(destino), "%s\\edge_y.bmp", dir_img);
            mover_archivo("imagen_edge_y.bmp", destino);
        }

        imagenes_procesadas++;
        printf("Imagen%d terminada.\n\n", idx_img);
    }

    fin = omp_get_wtime();

    printf("Proceso completado.\n");
    printf("Imagenes procesadas: %d de %d\n", imagenes_procesadas, TOTAL_IMAGENES);
    printf("Hilos solicitados: %d\n", NUM_THREADS);
    printf("Hilos disponibles en el ultimo equipo paralelo: %d\n", hilos_usados);
    printf("Tiempo transcurrido: %.4f segundos\n", fin - inicio);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "transformaciones.h"

#define NUM_THREADS 4
#define TOTAL_IMAGENES 10

int main(void)
{
    // Eliminar resultados anteriores
    system("if exist \"resultados\" rmdir /s /q \"resultados\"");
    system("del /q imagen_*.bmp 2>nul");

    // Crear carpeta principal
    system("mkdir \"resultados\"");

    char *imagenes[TOTAL_IMAGENES] = {
        "./10 imagenes/Imagen1.bmp",
        "./10 imagenes/Imagen2.bmp",
        "./10 imagenes/Imagen3.bmp",
        "./10 imagenes/Imagen4.bmp",
        "./10 imagenes/Imagen5.bmp",
        "./10 imagenes/Imagen6.bmp",
        "./10 imagenes/Imagen7.bmp",
        "./10 imagenes/Imagen8.bmp",
        "./10 imagenes/Imagen9.bmp",
        "./10 imagenes/Imagen10.bmp"
    };

    omp_set_num_threads(NUM_THREADS);

    double inicio = omp_get_wtime();

    printf("Procesando las 10 imagenes y organizando carpetas\n\n");

    for (int k = 0; k < TOTAL_IMAGENES; k++) {

        printf("Procesando [%d/%d]: %s\n",
               k + 1, TOTAL_IMAGENES, imagenes[k]);

        // Crear subcarpeta para cada imagen
        char comando_mkdir[100];

        snprintf(comando_mkdir, sizeof(comando_mkdir),
                 "mkdir \"resultados\\Imagen%d\"", k + 1);

        system(comando_mkdir);

        // Procesamiento paralelo
        #pragma omp parallel
        {
            #pragma omp sections
            {
                #pragma omp section
                inv_hor_color(imagenes[k]);

                #pragma omp section
                inv_hor_bn(imagenes[k]);

                #pragma omp section
                inv_ver_color(imagenes[k]);

                #pragma omp section
                inv_ver_bn(imagenes[k]);
            }
        }

        // Mover las imagenes procesadas
        char comando_mv[600];

        snprintf(comando_mv, sizeof(comando_mv),
            "move /Y \"imagen_horizontal_color.bmp\" \"resultados\\Imagen%d\\hor_color.bmp\" && "
            "move /Y \"imagen_horizontal_bn.bmp\" \"resultados\\Imagen%d\\hor_bn.bmp\" && "
            "move /Y \"imagen_vertical_color.bmp\" \"resultados\\Imagen%d\\ver_color.bmp\" && "
            "move /Y \"imagen_vertical_bn.bmp\" \"resultados\\Imagen%d\\ver_bn.bmp\"",
            k + 1, k + 1, k + 1, k + 1);

        if (system(comando_mv) != 0) {
            printf("Error al mover resultados de Imagen%d\n", k + 1);
        }
    }

    system("del /q imagen_*.bmp 2>nul");

    double fin = omp_get_wtime();
    double tiempo_total = fin - inicio;

    printf("\nProceso completado.\n");
    printf("Tiempo total de ejecucion: %f segundos\n", tiempo_total);

    return 0;
}

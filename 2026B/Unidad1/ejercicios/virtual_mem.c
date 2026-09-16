/* ============================================================================
 * SISTEMAS OPERATIVOS
 * Ejercicio 2 (PLANTILLA): Memoria virtual - Paginacion
 * Traduccion de direcciones logicas a fisicas y reemplazo de paginas (FIFO)
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#define TAM_PAGINA   256   /* bytes por pagina/marco                */
#define NUM_PAGINAS  8     /* paginas logicas del proceso           */
#define NUM_MARCOS   4     /* marcos fisicos disponibles (< NUM_PAGINAS) */

/* Tabla de paginas: tabla_paginas[i] = marco fisico donde esta cargada
 * la pagina logica i, o -1 si la pagina no esta actualmente en memoria. */
int tabla_paginas[NUM_PAGINAS];

/* Cola FIFO de marcos: guarda, en orden de llegada, que pagina logica
 * ocupa cada marco fisico. Se usa para saber cual reemplazar primero. */
int cola_marcos[NUM_MARCOS];   /* cola_marcos[m] = pagina logica en el marco m, o -1 si el marco esta libre */
int marcos_ocupados = 0;       /* cuantos marcos estan actualmente ocupados */
int siguiente_marco_a_reemplazar = 0; /* indice circular FIFO: proximo marco a liberar cuando todos esten llenos */

/* Contadores para estadisticas */
int contador_fallos = 0;
int contador_aciertos = 0;

/* ----------------------------------------------------------------------
 * inicializar_tabla_paginas
 * ---------------------------------------------------------------------- */
void inicializar_tabla_paginas(void) {
    int i;
    for (i = 0; i < NUM_PAGINAS; i++) {
        tabla_paginas[i] = -1;
    }
    for (i = 0; i < NUM_MARCOS; i++) {
        cola_marcos[i] = -1;
    }
    marcos_ocupados = 0;
    siguiente_marco_a_reemplazar = 0;
    contador_fallos = 0;
    contador_aciertos = 0;
}

/* ----------------------------------------------------------------------
 * mostrar_tabla_paginas
 * ---------------------------------------------------------------------- */
void mostrar_tabla_paginas(void) {
    int i;
    printf("\n--- Tabla de paginas (proceso) ---\n");
    for (i = 0; i < NUM_PAGINAS; i++) {
        if (tabla_paginas[i] == -1) {
            printf(" Pagina %d -> no cargada\n", i);
        } else {
            printf(" Pagina %d -> Marco %d\n", i, tabla_paginas[i]);
        }
    }
    printf("--- Marcos fisicos ---\n");
    for (i = 0; i < NUM_MARCOS; i++) {
        if (cola_marcos[i] == -1) {
            printf(" Marco %d -> libre\n", i);
        } else {
            printf(" Marco %d -> Pagina %d\n", i, cola_marcos[i]);
        }
    }
    printf("Fallos de pagina: %d | Aciertos: %d\n", contador_fallos, contador_aciertos);
}

/* ----------------------------------------------------------------------
 * cargar_pagina_en_memoria
 * ---------------------------------------------------------------------- */
int cargar_pagina_en_memoria(int pagina) {
    int marco_elegido;
    
    if (marcos_ocupados < NUM_MARCOS) {
        /* Aun hay marcos libres */
        marco_elegido = marcos_ocupados;
        marcos_ocupados++;
    } else {
        /* Todos los marcos ocupados: aplicar reemplazo FIFO */
        marco_elegido = siguiente_marco_a_reemplazar;
        
        /* Obtener la pagina antigua que estaba en este marco para invalidarla */
        int pagina_antigua = cola_marcos[marco_elegido];
        if (pagina_antigua != -1) {
            tabla_paginas[pagina_antigua] = -1;
        }
        
        /* Avanzar el puntero FIFO de forma circular */
        siguiente_marco_a_reemplazar = (siguiente_marco_a_reemplazar + 1) % NUM_MARCOS;
    }
    
    /* Asignar el nuevo marco a la pagina solicitada */
    cola_marcos[marco_elegido] = pagina;
    tabla_paginas[pagina] = marco_elegido;
    
    return marco_elegido;
}

/* ----------------------------------------------------------------------
 * traducir_direccion
 * ---------------------------------------------------------------------- */
int traducir_direccion(int direccion_logica) {
    int numero_pagina = direccion_logica / TAM_PAGINA;
    int desplazamiento = direccion_logica % TAM_PAGINA;
    int marco;
    int direccion_fisica;

    /* Validar que el numero de pagina este en rango */
    if (numero_pagina < 0 || numero_pagina >= NUM_PAGINAS) {
        printf("Direccion logica %d invalida. El numero de pagina %d esta fuera de rango (0-%d).\n", 
               direccion_logica, numero_pagina, NUM_PAGINAS - 1);
        return -1;
    }

    if (tabla_paginas[numero_pagina] != -1) {
        /* ACIERTO (Hit) */
        marco = tabla_paginas[numero_pagina];
        contador_aciertos++;
        printf("[ACIERTO] ");
    } else {
        /* FALLO DE PAGINA (Miss) */
        contador_fallos++;
        printf("[FALLO]   ");
        marco = cargar_pagina_en_memoria(numero_pagina);
    }

    /* Calcular direccion fisica */
    direccion_fisica = (marco * TAM_PAGINA) + desplazamiento;

    printf("Direccion logica %d -> pagina %d, desplazamiento %d -> "
           "marco %d -> direccion fisica %d\n",
           direccion_logica, numero_pagina, desplazamiento, marco, direccion_fisica);

    return direccion_fisica;
}

/* ----------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main() {
    /* Secuencia de direcciones logicas que "el proceso" va accediendo.
     * Con TAM_PAGINA = 256, la direccion 300 corresponde a la pagina 1
     * (300 / 256 = 1) con desplazamiento 44 (300 % 256 = 44), etc. */
    int secuencia[] = {0, 300, 600, 900, 1200, 1500, 50, 1800, 610, 2000};
    int total = sizeof(secuencia) / sizeof(secuencia[0]);
    int i, opcion, direccion;

    inicializar_tabla_paginas();

    printf("=============================================================\n");
    printf(" SIMULADOR DE MEMORIA VIRTUAL - PAGINACION CON REEMPLAZO FIFO\n");
    printf(" Tamano de pagina: %d bytes | Paginas logicas: %d | Marcos: %d\n",
           TAM_PAGINA, NUM_PAGINAS, NUM_MARCOS);
    printf("=============================================================\n");

    do {
        printf("\n1. Ejecutar secuencia de accesos de ejemplo\n");
        printf("2. Traducir una direccion logica manualmente\n");
        printf("3. Ver tabla de paginas y marcos\n");
        printf("0. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                for (i = 0; i < total; i++) {
                    traducir_direccion(secuencia[i]);
                }
                mostrar_tabla_paginas();
                break;
            case 2:
                printf("Ingrese direccion logica (0 - %d): ", TAM_PAGINA * NUM_PAGINAS - 1);
                scanf("%d", &direccion);
                traducir_direccion(direccion);
                break;
            case 3:
                mostrar_tabla_paginas();
                break;
            case 0:
                printf("Saliendo del simulador...\n");
                break;
            default:
                printf("Opcion invalida.\n");
        }
    } while (opcion != 0);

    return 0;
}

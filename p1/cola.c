#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cola.h"

void inicializar_cola(Cola *cola, int tam_cola)
{
    register int i;

    if (tam_cola < 1)
    {
        perror("Error: El tamaño de la cola debe ser mayor o igual que 1");
        exit(1);
    }
    if (cola == NULL)
    {
        perror("Error: El puntero a la cola es NULL en inicializar_cola");
        exit(2);
    }

    // A RELLENAR el resto de la inicialización de la cola
    cola -> datos = (dato_cola **) malloc(tam_cola * sizeof(dato_cola *));
    if (cola -> datos == NULL) {
        perror("malloc cola->datos");
        exit(EXIT_FAILURE);
    }

    cola -> head = 0;
    cola -> tail = 0;
    cola -> tam_cola = tam_cola;

    if (pthread_mutex_init(&(cola -> mutex_head), NULL) != 0) {
        perror("Pthread init mutex_head");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&(cola -> mutex_tail), NULL) != 0) {
        perror("Pthread init mutex_tail");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&(cola -> num_huecos), 0, tam_cola) != 0) {
        perror("Semaphore init num_huecos");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&(cola -> num_ocupados), 0, 0) != 0) {
        perror("Semaphore init num_ocupados");
        exit(EXIT_FAILURE);
    }
}

void destruir_cola(Cola *cola)
{
    // Debe liberarse la memoria apuntada por cada puntero guardado en la cola
    // y la propia memoria donde se guardan esos punteros, así como
    // destruir los semáforos y mutexes
    
    // A RELLENAR
    free(cola -> datos);
    if (pthread_mutex_destroy(&(cola -> mutex_head)) != 0) {
        perror("Pthread destroy mutex_head");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_destroy(&(cola -> mutex_tail)) != 0) {
        perror("Pthread destroy mutex_tail");
        exit(EXIT_FAILURE);
    }

    if (sem_destroy(&(cola -> num_huecos)) != 0) {
        perror("Semaphore destroy num_huecos");
        exit(EXIT_FAILURE);
    }

    if (sem_destroy(&(cola -> num_ocupados)) != 0) {
        perror("Semaphore destroy num_ocupados");
        exit(EXIT_FAILURE);
    }

}

void insertar_dato_cola(Cola *cola, dato_cola *dato)
{
    // A RELLENAR
    if (cola == NULL) {
        fprintf(stderr, "cola == NULL\n");
        exit(EXIT_FAILURE);
    }

    if (cola -> datos == NULL) {
        fprintf(stderr, "cola->datos == NULL\n");
        exit(EXIT_FAILURE);
    }

    if (sem_wait(&(cola -> num_huecos)) != 0) {
        perror("sem_wait num_huecos");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_lock(&(cola -> mutex_head)) != 0) {
        perror("pthread_mutex_lock mutex_head");
        exit(EXIT_FAILURE);
    }

    cola -> datos[cola -> head] = dato;
    cola -> head = (cola -> head + 1) % cola -> tam_cola;

    if (pthread_mutex_unlock(&(cola -> mutex_head)) != 0) {
        perror("pthread_mutex_unlock mutex_head");
        exit(EXIT_FAILURE);
    }

    if (sem_post(&(cola -> num_ocupados)) != 0) {
        perror("sem_post num_ocupados");
        exit(EXIT_FAILURE);
    }

}

dato_cola *obtener_dato_cola(Cola *cola)
{
    dato_cola *p;

    // A RELLENAR
    if (cola == NULL) {
        fprintf(stderr, "cola == NULL\n");
        exit(EXIT_FAILURE);
    }

    if (cola -> datos == NULL) {
        fprintf(stderr, "cola->datos == NULL\n");
        exit(EXIT_FAILURE);
    }

    if (sem_wait(&(cola -> num_ocupados)) != 0) {
        perror("sem_wait num_ocupados");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_lock(&(cola -> mutex_tail)) != 0) {
        perror("pthread_mutex_lock mutex_tail");
        exit(EXIT_FAILURE);
    }

    p = cola -> datos[cola -> tail];
    cola -> tail = (cola -> tail + 1) % cola -> tam_cola;

    if (pthread_mutex_unlock(&(cola -> mutex_tail)) != 0) {
        perror("pthread_mutex_unlock mutex_tail");
        exit(EXIT_FAILURE);
    }

    if (sem_post(&(cola -> num_huecos)) != 0) {
        perror("sem_post num_huecos");
        exit(EXIT_FAILURE);
    }

    return p;
}


void imprimir_dato_cola(dato_cola *dato) {
    printf("Contenido de dato_cola:\n");
    printf(" - Mensaje: %s", dato->msg);
    printf(" - Socket: %d\n", dato->s);
    printf(" - Dirección IP del cliente: %s\n", inet_ntoa(dato->d_cliente.sin_addr));
    printf(" - Puerto del cliente: %d\n", ntohs(dato->d_cliente.sin_port));
}
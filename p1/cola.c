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
    cola->head = 0;
    cola->tail = 0;
    cola->tam_cola = tam_cola;

    cola->datos = (dato_cola **)malloc(tam_cola * sizeof(dato_cola *));

    if(cola->datos == NULL){
        perror("Error: No se pudo asignar memoria para la cola");
        exit(3);
    }

    for(i = 0; i < tam_cola; ++i){
        cola->datos[i] = (dato_cola *)malloc(sizeof(dato_cola));
        if(cola->datos[i] == NULL){
            perror("Error: No se pudo asignar memoria para el elemento de la cola");
            exit(4);
        }
    }

    pthread_mutex_init(&cola->mutex_head, NULL);
    pthread_mutex_init(&cola->mutex_tail, NULL);
    sem_init(&cola->num_huecos, 0, tam_cola);
    sem_init(&cola->num_ocupados, 0, 0);
}

void destruir_cola(Cola *cola)
{
    // Debe liberarse la memoria apuntada por cada puntero guardado en la cola
    // y la propia memoria donde se guardan esos punteros, así como
    // destruir los semáforos y mutexes
    
    // A RELLENAR
    for (int i = 0; i < cola->tam_cola; ++i)
    {
        free(cola->datos[i]);
    }

    free(cola->datos);


    pthread_mutex_destroy(&cola->mutex_head);
    pthread_mutex_destroy(&cola->mutex_tail);
    sem_destroy(&cola->num_huecos);
    sem_destroy(&cola->num_ocupados);

}

void insertar_dato_cola(Cola *cola, dato_cola *dato)
{
    // A RELLENAR
    if (cola == NULL || dato == NULL)
    {
        perror("Error: El puntero a la cola o al dato es NULL en insertar_dato_cola");
        exit(1);
    }

    sem_wait(&cola->num_huecos); // Esperar hasta que haya espacio disponible en la cola
    pthread_mutex_lock(&cola->mutex_tail); // Bloquear el acceso al tail de la cola

    // Insertar el dato en la posición tail
    cola->datos[cola->tail]->s = dato->s;
    strcpy(cola->datos[cola->tail]->msg, dato->msg);
    memcpy(&cola->datos[cola->tail]->d_cliente, &dato->d_cliente, sizeof(struct sockaddr_in));

    // Mover el tail al siguiente elemento circularmente
    cola->tail = (cola->tail + 1) % cola->tam_cola;

    pthread_mutex_unlock(&cola->mutex_tail); // Desbloquear el acceso al tail de la cola
    sem_post(&cola->num_ocupados); // Incrementar el contador de elementos ocupados en

}

dato_cola *obtener_dato_cola(Cola *cola)
{
    dato_cola *p;

    // A RELLENAR
    if (cola == NULL)
    {
        perror("Error: El puntero a la cola es NULL en obtener_dato_cola");
        exit(1);
    }

    sem_wait(&cola->num_ocupados); // Esperar hasta que haya datos disponibles en la cola
    pthread_mutex_lock(&cola->mutex_head); // Bloquear el acceso al head de la cola

    // Obtener el dato en la posición head
    p = cola->datos[cola->head];

    // Mover el head al siguiente elemento circularmente
    cola->head = (cola->head + 1) % cola->tam_cola;

    pthread_mutex_unlock(&cola->mutex_head); // Desbloquear el acceso al head de la cola
    sem_post(&cola->num_huecos); // Incrementar el contador de huecos disponibles en la cola

    return p;
}

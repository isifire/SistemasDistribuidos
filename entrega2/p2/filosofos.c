/*****************************************************************************

                               filosofos.c

En esta version distribuida del problema de los filosofos requiere de un
maestro que controle los recursos compartidos (palillos).

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Funciones para simplificar la API de sockets. Se declaran aquí
// pero no se implementan. El alumno puede copiar su implementación
// de otros ejercicios, o implementarlas él mismo

// Crea un socket y si no hay errores lo retorna
int CrearSocketClienteTCP(void);

// Conecta a la IP y puertos dados. La IP se le pasa como cadena
// Si hay errores termina el programa
void ConectarConServidor(int s, char *ip, int puerto);

// Las dos siguientes funciones equivalen a send() y recv() pero
// si detectan un error terminan el programa
int Enviar(int s, char *buff, int longitud);
int Recibir(int s, char *buff, int longitud);

// Cierra el socket
void CerrarSocket(int s);

#define MAX_BOCADOS 10
#define LOCK 1
#define UNLOCK 2

// Prototipos de las funciones
void filosofo(int numfilo, char *ip, int puerto, int maxfilo);
int controlMutex(char *ipmaster, int puerto, char op, int nummutex);
int controlMutexDuo(char *ipmaster, int puerto, char op, int nummutex1, int nummutex2);


void log_debug(char *msg)   //FASE 2
{
    struct timespec t;
    clock_gettime(_POSIX_MONOTONIC_CLOCK, &t);

    printf("[%ld.%09ld] %s", t.tv_sec, t.tv_nsec, msg);
}

void main(int argc, char *argv[])
{
    setbuf(stdout, NULL);   //FASE 2
    int numfilo;
    int puerto;
    int maxfilo;

    // Comprobacion del número de argumentos
    if (argc < 5)
    {
        fprintf(stderr, "Forma de uso: %s numfilo ip_maestro puerto maxfilo\n",
                argv[0]);
        exit(1);
    }
    /*** Lectura de valores de los argumentos ***/
    numfilo = atoi(argv[1]);
    if ((numfilo < 0) || (numfilo > 4))
    {
        fprintf(stderr, "El numero de filosofo debe ser >=0 y <=4\n");
        exit(2);
    }
    puerto = atoi(argv[3]);
    if ((puerto < 0) || (puerto > 65535)) //debería ser puerto < 1024
    {
        fprintf(stderr, "El numero de puerto debe ser >=0 y <=65535\n");
        exit(3);
    }
    maxfilo = atoi(argv[4]);
    if (maxfilo < 0)
    {
        fprintf(stderr, "El numero de filosofos debe ser mayor que 0\n");
        exit(3);
    }
    /*********************************************/
    // lanzamiento del filósofo
    filosofo(numfilo, argv[2], puerto, maxfilo);
    exit(0);
}

// función que implementa las tareas a realizar por el filósofo
void filosofo(int numfilo, char *ip, int puerto, int maxfilo)
{
    int veces = 0;

    char msg[100]; // Para almacenar el mensaje de log

    sprintf(msg, "Filosofo %d sentado a la mesa.\n", numfilo);
    log_debug(msg);
    while (veces < MAX_BOCADOS)
    {
        // mientras el filósofo no logre obtener ambos palillos, seguirá intentándolo
        while (controlMutexDuo(ip, puerto, LOCK, numfilo, (numfilo + 1) % maxfilo) == 0) //FASE 3
            ;
        
        // ya tiene ambos palillos y puede comer
        sprintf(msg, "El filosofo %d toma palillos %d, %d y esta comiendo.\n", numfilo, numfilo, (numfilo + 1) % maxfilo); //FASE 2
        log_debug(msg);
        sleep(3);
        
        // libera ambos palillos
        while (controlMutexDuo(ip, puerto, UNLOCK, numfilo, (numfilo + 1) % maxfilo) == 0) //FASE 3
            ;
        
        // el filósofo ha soltado ambos palillos y puede dedicarse a pensar
        sprintf(msg, "El filosofo %d deja palillos %d, %d y esta pensando.\n", numfilo, numfilo, (numfilo + 1) % maxfilo); //FASE 2
        log_debug(msg);
        sleep(5);
        
        // incrementamos el número de veces que el filósofo ha podido completar el ciclo
        veces++;
    }
    // el filósofo ha completado el número de bocados y se levanta de la mesa
    sprintf(msg, "El filosofo %d se ha levantado de la mesa.\n", numfilo);
    log_debug(msg);
}

// Función similar a controlMutex pero que maneja dos palillos simultáneamente
int controlMutexDuo(char *ipmaster, int puerto, char op, int nummutex1, int nummutex2) //FASE 3
{
    int sock;
    char buffer[20];
    char *ptr;

    sock = CrearSocketClienteTCP();
    ConectarConServidor(sock, ipmaster, puerto);
    switch (op)
    {
    case LOCK:
        sprintf(buffer, "L %d %d", nummutex1, nummutex2);
        break;
    case UNLOCK:
        sprintf(buffer, "U %d %d", nummutex1, nummutex2);
        break;
    }
    Enviar(sock, buffer, strlen(buffer));
    sprintf(buffer, "%c", '\0');
    Recibir(sock, buffer, 10);
    ptr = strtok(buffer, " \t\n");
    CerrarSocket(sock);
    return (atoi(ptr));
}


// esta función le permite al filósofo solicitar-liberar un recurso
// gestionado por el coordinador
int controlMutex(char *ipmaster, int puerto, char op, int nummutex)
{
    // Necesita de las funciones de libsokets
    int sock;
    char buffer[10];
    char *ptr;

    // crea el socket de conexion con el maestro de recursos
    sock = CrearSocketClienteTCP();
    // abre conexión con el maestro de recursos
    ConectarConServidor(sock, ipmaster, puerto);
    // en buffer confecciona el mensaje en función de la operación
    // a realizar y el palillo a solicitar (LOCK) o liberar (UNLOCK)
    sprintf(buffer, "%c", '\0');
    switch (op)
    {
    case LOCK:
        sprintf(buffer, "L %d", nummutex);
        break;
    case UNLOCK:
        sprintf(buffer, "U %d", nummutex);
        break;
    }
    // se envía el mensaje al maestro de recursos
    Enviar(sock, buffer, strlen(buffer));
    // se inicializa el buffer para la recepción
    sprintf(buffer, "%c", '\0');
    // se espera síncronamente la respuesta del mensaje
    Recibir(sock, buffer, 10);
    // se obtiene en ptr un puntero al primer token del mensaje
    ptr = strtok(buffer, " \t\n");
    // se cierra el socket de comunicación con el servidor
    CerrarSocket(sock);
    // se retorna el valor entero del token recibido
    // 1-OK, 0-NO OK
    return (atoi(ptr));
}

int CrearSocketClienteTCP(void) //FASE 1
{
    int s;

    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        perror("socket");
        exit(1);
    }
    return s;
}

void ConectarConServidor(int s, char *ip, int puerto) //FASE 1
{
    struct sockaddr_in dir;

    dir.sin_family = AF_INET;
    dir.sin_port = htons(puerto);
    dir.sin_addr.s_addr = inet_addr(ip);
    if (connect(s, (struct sockaddr *)&dir, sizeof(dir)) == -1)
    {
        perror("connect");
        exit(1);
    }
}


int Enviar(int s, char *buff, int longitud) //FASE 1
{
    int ret;

    ret = send(s, buff, longitud, 0);
    if (ret == -1)
    {
        perror("send");
        exit(1);
    }
    return ret;
}

int Recibir(int s, char *buff, int longitud)    //FASE 1
{
    int ret;

    ret = recv(s, buff, longitud, 0);
    if (ret == -1)
    {
        perror("recv");
        exit(1);
    }
    return ret;
}

void CerrarSocket(int s)    //FASE 1
{
    close(s);
}

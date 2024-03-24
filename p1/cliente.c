// Archivos de cabecera para manipulación de sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include "util.h"

#define TAMLINEA 1024
#define SINASIGNAR -1
#define MAXHILOSCLIENTE 10

// tipo de datos que recibiran los hilos cliente
struct datos_hilo
{
    unsigned char id;
    char *nom_fichero_consultas;
    struct sockaddr *dserv;
};

typedef struct datos_hilo datos_hilo;

//
// VARIABLES GLOBALES
//

// IP del proceso srvdns
char *ip_srvdns;

// Puerto en el que espera el proceso srvdns
int puerto_srvdns;

// Numero de hilos lectores
int nhilos;

// Es o no orientado a conexion
unsigned char es_stream = CIERTO;

// nombre del fichero fuente de consultas
char *fich_consultas;

char *hilos_file_names[MAXHILOSCLIENTE] = {
    "sal00.dat",
    "sal01.dat",
    "sal02.dat",
    "sal03.dat",
    "sal04.dat",
    "sal05.dat",
    "sal06.dat",
    "sal07.dat",
    "sal08.dat",
    "sal09.dat"};

void procesa_argumentos(int argc, char *argv[])
{
    if (argc < 6)
    {
        fprintf(stderr, "Forma de uso: %s ip_srvdns puerto_srvdns {t|u} nhilos fich_consultas\n", argv[0]);
        exit(1);
    }
    // Verificación de los argumentos e inicialización de las correspondientes variables globales.
    // Puedes usar las funciones en util.h

    // A RELLENAR
    ip_srvdns = argv[1];    
    nhilos = atoi(argv[4]);
    fich_consultas = argv[5];

    char *ip_cpy = malloc(strlen(ip_srvdns) + 1);
    strcpy(ip_cpy,ip_srvdns);

    if (!valida_ip(ip_cpy)){
        fprintf(stderr, "La dirección IP del servidor NO es válida.\n");
        exit(1);
    }

    if(!valida_numero(argv[2])){
        fprintf(stderr, "El puerto no es un número.\n");
        exit(2);
    }

    else{
        puerto_srvdns = atoi(argv[2]);
    }


    if (puerto_srvdns < 1023 || puerto_srvdns > 65535)
    {
        fprintf(stderr, "El puerto especificado no está en un rango válido.\n");
        exit(3);
    }

    if (nhilos < 1 || nhilos > MAXHILOSCLIENTE)
    {
        fprintf(stderr, "El número de hilos debe estar entre 1 y %d.\n", MAXHILOSCLIENTE);
        exit(4);
    }

    if(fopen(fich_consultas, "r") == NULL){
        fprintf(stderr, "El fichero de registros dns NO existe");
        exit(5);
    }

    if(strcmp(argv[3], "t") == 0)
    {
        es_stream = CIERTO;
    }
    else if(strcmp(argv[3], "u") == 0)
    {
        es_stream = FALSO;
    }
    else
    {
        fprintf(stderr, "El segundo argumento debe ser 't' o 'u'\n");
        exit(2);
    }
}

void salir_bien(int s)
{
    exit(0);
}

void *hilo_lector(datos_hilo *p)
{
    int enviados, recibidos;
    char buffer[TAMLINEA];
    char respuesta[TAMLINEA];
    char *s;
    int sock_dat;
    FILE *fpin;
    FILE *fpout;

    if ((fpin = fopen(p->nom_fichero_consultas, "r")) == NULL)
    {
        perror("Error: No se pudo abrir el fichero de consultas");
        pthread_exit(NULL);
    }
    if ((fpout = fopen(hilos_file_names[p->id], "w")) == NULL)
    {
        fclose(fpin); // cerramos el handler del fichero de consultas
        perror("Error: No se pudo abrir el fichero de resultados");
        pthread_exit(NULL);
    }
    do
    {
        bzero(buffer, TAMLINEA);
        s = fgets(buffer, TAMLINEA, fpin);

        if (s != NULL)
        {
            if (es_stream)
            {
                // Enviar el mensaje leído del fichero a través de un socket TCP
                // y leer la respuesta del servidor
                // A RELLENAR
                sock_dat = socket(AF_INET, SOCK_STREAM, 0);
				
                if (sock_dat < 0)
                {
                    perror("Error al crear el socket");
                    pthread_exit(NULL);
                }
       
                if (connect(sock_dat, p->dserv, sizeof(struct sockaddr)) < 0)
                {
                    perror("Error al conectar al servidor");
                    pthread_exit(NULL);
                }

                enviados = write(sock_dat, buffer, strlen(buffer));
                if (enviados < 0)
                {
                    perror("Error al escribir en el socket");
                    pthread_exit(NULL);
                }

                bzero(respuesta, TAMLINEA);
                recibidos = read(sock_dat, respuesta, TAMLINEA);
                if (recibidos < 0)
                {
                    perror("Error al leer desde el socket");
                    pthread_exit(NULL);
                }
            }
            else
            {
                // Enviar el mensaje leído del fichero a través de un socket UDP
                // y leer la respuesta del servidor
                // A RELLENAR
                sock_dat = socket(AF_INET, SOCK_DGRAM, 0);
                if (sock_dat < 0)
                {
                    perror("Error al crear el socket");
                    pthread_exit(NULL);
                }

                if (sendto(sock_dat, buffer, strlen(buffer), 0, p->dserv, sizeof(struct sockaddr)) < 0) 
                {
                    perror("Error al enviar datos por UDP");
                    pthread_exit(NULL);
                }

                bzero(respuesta, TAMLINEA);
                socklen_t addr_len = sizeof(struct sockaddr);
                if (recvfrom(sock_dat, respuesta, TAMLINEA, 0, p->dserv, &addr_len) < 0)
                {
                    perror("Error al recibir datos por UDP");
                    pthread_exit(NULL);
                }
            }
            close(sock_dat);
            // Volcar la petición y la respuesta, separadas por ":" en
            // el fichero de resultados
            // A RELLENAR
            size_t len_buffer = strlen(buffer);
            if (len_buffer > 0 && buffer[len_buffer - 1] == '\n')
                buffer[len_buffer - 1] = '\0';

            size_t len_respuesta = strlen(respuesta);
            if (len_respuesta > 0 && respuesta[len_respuesta - 1] == '\n')
                respuesta[len_respuesta - 1] = '\0';

            // Escribir en el archivo de salida con un solo salto de línea al final
            fprintf(fpout, "%s:%s\n", buffer, respuesta);
        }
    } while (s);
    // Terminado el hilo, liberamos la memoria del puntero y cerramos ficheros
    fclose(fpin);
    fclose(fpout);
    free(p);
    return NULL;
}

int main(int argc, char *argv[])
{
    register int i;

    pthread_t *th;
    datos_hilo *q;

    struct sockaddr_in d_serv;

    // handler de archivo
    FILE *fp;

    signal(SIGINT, salir_bien);
    procesa_argumentos(argc, argv);

    // Comprobar si se puede abrir el fichero, para evitar errores en los hilos
    if ((fp = fopen(fich_consultas, "r")) == NULL)
    {
        perror("Error: No se pudo abrir el fichero de consultas");
        exit(6);
    }
    else
        fclose(fp); // cada hilo lo abrirá para procesarlo

    if ((th = (pthread_t *)malloc(sizeof(pthread_t) * nhilos)) == NULL)
    {
        fprintf(stderr, "No se pudo reservar memoria para los objetos de datos de hilo\n");
        exit(7);
    }

    // Inicializar la estructura de dirección del servidor que se pasará a los hilos
    // A RELLENAR
    memset(&d_serv, 0, sizeof(struct sockaddr_in));
    d_serv.sin_family = AF_INET;
    d_serv.sin_addr.s_addr = inet_addr(ip_srvdns);
    d_serv.sin_port = htons(puerto_srvdns);

    for (i = 0; i < nhilos; i++)
    {
        // Preparar el puntero con los parámetros a pasar al hilo
        // A RELLENAR
        q = (datos_hilo *)malloc(sizeof(datos_hilo));
        q->id = i;
        q->nom_fichero_consultas = fich_consultas;
        q->dserv = (struct sockaddr *)&d_serv;
        // Crear el hilo
        // A RELLENAR
        if ( pthread_create(&th[i], NULL, (void *)hilo_lector, (void *)q) != 0)
        {
            fprintf(stderr, "No se pudo crear el hilo lector %d\n", i);
            exit(9);
        }
    }
    
    // Esperar a que terminen todos los hilos
    for (i = 0; i < nhilos; i++)
    {
        pthread_join(th[i], NULL);
    }
}

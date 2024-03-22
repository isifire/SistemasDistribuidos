#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_PENDING_CONNECTIONS 10

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Crear el socket TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }
    
    // Configurar opciones del socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Error al configurar el socket");
        exit(EXIT_FAILURE);
    }
    
    // Configurar la dirección y el puerto en el que el servidor va a escuchar
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Vincular el socket a la dirección y el puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error al vincular el socket");
        exit(EXIT_FAILURE);
    }
    
    // Escuchar conexiones entrantes
    if (listen(server_fd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("Error al escuchar conexiones");
        exit(EXIT_FAILURE);
    }
    
    // Aceptar conexiones entrantes
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Error al aceptar la conexión");
        exit(EXIT_FAILURE);
    }
    
    printf("Cliente conectado\n");
    
    // Aquí puedes continuar con la lógica para manejar la conexión entrante
    
    close(new_socket); // Cerrar el socket de la conexión
    close(server_fd);  // Cerrar el socket del servidor
    
    return 0;
}
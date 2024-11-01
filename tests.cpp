#include <iostream>
#include <cstring>
#include <unistd.h>       // Para close(), read(), write() y otras funciones del sistema.
#include <arpa/inet.h>    // Para estructuras y funciones de redes (sockaddr_in, htons).
#include <fcntl.h>        // Para configurar las propiedades del socket, como la opción no-bloqueante.
#include <poll.h>         // Para la función poll(), que nos permite esperar eventos en varios sockets.
#include <vector>         // Para manejar dinámicamente nuestros clientes.


#define PORT 8080
#define MAX_CLIENTS 100  //Preguntar para que sirve esto, que es un cliente.
#define BUFFER_SIZE 1024 //¿Qué datos?

void setNonBlocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);      // Obtener los flags actuales del socket.
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); // Añadir la bandera O_NONBLOCK.
}

int main()
{
    int server_fd, new_socket;  //¿Que es un socket?
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    std::vector<pollfd> poll_fds;

    // Crear socket del servidor
    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar socket del servidor
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Error en setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error en bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Error en listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Hacer no bloqueante el socket del servidor y agregarlo a poll_fds
    setNonBlocking(server_fd);
    poll_fds.push_back({server_fd, POLLIN, 0});

    std::cout << "Servidor escuchando en el puerto " << PORT << "...\n";

    while (true) {
        int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
        if (poll_count < 0) {
            perror("Error en poll");
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == server_fd) {
                    // Nueva conexión entrante
                    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                        perror("Error en accept");
                        continue;
                    }
                    setNonBlocking(new_socket);
                    poll_fds.push_back({new_socket, POLLIN | POLLOUT, 0});
                    std::cout << "Nueva conexión aceptada.\n";
                } else {
                    // Leer datos de un cliente
                    memset(buffer, 0, BUFFER_SIZE);
                    int valread = read(poll_fds[i].fd, buffer, BUFFER_SIZE);
                    if (valread <= 0) {
                        close(poll_fds[i].fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        std::cout << "Conexión cerrada por el cliente.\n";
                        --i;
                    } else {
                        std::cout << "Mensaje recibido: " << buffer;

                        // Enviar una respuesta HTTP bien formada al cliente
                        std::string response = "HTTP/1.1 200 OK\r\n"
                                               "Content-Type: text/plain\r\n"
                                               "Content-Length: 13\r\n"
                                               "\r\n"
                                               "Respuesta bien bacana";
                        write(poll_fds[i].fd, response.c_str(), response.size());
                    }
                }
            }
        }
    }

    // Cerrar sockets
    for (auto& pfd : poll_fds) {
        close(pfd.fd);
    }

    return 0;
}

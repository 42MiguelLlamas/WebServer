#include <iostream>
#include <cstring>
#include <unistd.h>       // Para close(), read(), write() y otras funciones del sistema.
#include <arpa/inet.h>    // Para estructuras y funciones de redes (sockaddr_in, htons).
#include <fcntl.h>        // Para configurar las propiedades del socket, como la opción no-bloqueante.
#include <poll.h>         // Para la función poll(), que nos permite esperar eventos en varios sockets.
#include <vector>         // Para manejar dinámicamente nuestros clientes.
#include <sstream>
#include <map>
#include <cstring>
#include <fstream>


#define PORT 8080
#define MAX_CLIENTS 100  //Preguntar para que sirve esto, que es un cliente.
#define BUFFER_SIZE 1024 //¿Qué datos?

int handle_error(int fd, std::string a)
{
    std::cerr << "Error en '" << a << "': " << strerror(errno) << std::endl;
    close(fd);
    return 1;
}

int setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);   // Obtiene los flags actuales del socket
    if (flags == -1)
        return handle_error(fd, "get flags");

    // Configura el socket para que sea no bloqueante
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return handle_error(fd, "set non-blocking");

    return 0;
}

std::string getHtmlContent(const std::string& path)
{
    std::string filePath = "./html" + path;
    if (filePath.back() == '/')
        filePath += "index.html"; // Si es la raíz o una carpeta, busca 'index.html'
    else
        filePath += ".html"; // Agrega la extensión .html a cualquier otra ruta

    // Intenta abrir el archivo
    std::ifstream file(filePath);
    if (!file.is_open()) // Si el archivo no existe, devolver mensaje de error 404
    {
        std::cout << "Archivo no encontrado" << std::endl;
        file.open("./html/404.html");
        if (!file.is_open())
        {
            std::cerr << "Error: No se pudo abrir 404.html tampoco." << std::endl;
            return "<html><body><h1>404 Not Found</h1><p>El recurso solicitado no fue encontrado en el servidor.</p></body></html>";
        }
    }
    
    // Lee el contenido del archivo en un string
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

void parseHttpRequest(const std::string& request, int client_fd)
{
    std::istringstream stream(request);
    std::string requestLine;
    std::getline(stream, requestLine); // Obtener la primera línea completa

    // Crear un flujo para analizar solo la línea de solicitud
    std::istringstream requestLineStream(requestLine);
    std::string method, path, httpVersion;
    requestLineStream >> method >> path >> httpVersion;

    // Imprimir cada parte de la línea de solicitud
    std::cout << "Request Line: " << requestLine << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "HTTP Version: " << httpVersion << std::endl;

    // Almacenar encabezados en un mapa
    std::map<std::string, std::string> headers;
    std::string headerLine;
    while (std::getline(stream, headerLine) && headerLine != "")
    {
        size_t separator = headerLine.find(':');
        if (separator != std::string::npos)
        {
            std::string key = headerLine.substr(0, separator);
            std::string value = headerLine.substr(separator + 1);
            headers[key] = value;
        }
    }

    // Manejar diferentes métodos
    if (method == "GET")
    {
        std::string htmlContent = getHtmlContent(path);
        std::string response;
        if (htmlContent.find("404 Not Found") != std::string::npos) // Respuesta 404
            response = "HTTP/1.1 404 Not Found\r\n";
        else // Respuesta 200
            response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: " + std::to_string(htmlContent.size()) + "\r\n";
        response += "\r\n";
        response += htmlContent;
        send(client_fd, response.c_str(), response.size(), 0);
    }
    else if (method == "POST")
    {
        std::cout << "Handling POST request." << std::endl;
    }
    else if (method == "DELETE")
    {
        std::cout << "Handling DELETE request." << std::endl;
    }
    else
    {
        std::string response = "HTTP/1.1 501 Not Implemented\r\n\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
}



int main()
{
    int server_fd;
    int new_socket;
    std::string buffer(1024, '\0');
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    std::vector<pollfd> fds(1);

    // Crear socket del servidor
    server_fd = socket(PF_INET, SOCK_STREAM, 0); //PF_Inet es para IPv4, 
    if (server_fd == -1)
        return handle_error(server_fd, "socket");

    // Configurar socket del servidor para poder ser reusado en on/off rapido
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
        return handle_error(server_fd, "set socket");

    address.sin_family = AF_INET; //Ipv4
    address.sin_addr.s_addr = INADDR_ANY; //Acepta conexiones de cualquier IP
    address.sin_port = htons(PORT); //Puerto en formato red.

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
        return handle_error(server_fd, "bind");

    if (listen(server_fd, 3) == -1) //3 Conex max en cola. SOMAXCONN if wanted all from oper.system
        return handle_error(server_fd, "listen");

    if (setNonBlocking(server_fd))
        return 1;  //fallo algó

    std::cout << "Servidor escuchando en el puerto " << PORT << "...\n";

    while (true)
    {
        pollfd server; // Solo un socket del servidor
        fds[0].fd = server_fd; // Socket del servidor
        fds[0].events = POLLIN; // Espera eventos de entrada (nueva conexión)
        fds.push_back(server);
        int ret = poll(fds.data(), fds.size(), -1); // Esperar indefinidamente por eventos
        if (ret == -1)
        {
            std::cerr << "Error en poll: " << strerror(errno) << std::endl;
            break; //Salir del bucle si hay error
        }
        if (fds[0].revents & POLLIN) //Operacion binaria, algun bit de revents coincide con POLLIN, es que el evento ocurrio.
        {
            new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket == -1)
            {
                std::cerr << "Error en accept: " << strerror(errno) << std::endl;
                continue; // Continuar con la siguiente iteración del bucle
            }
            std::cout << "Nueva conexión aceptada en el socket " << new_socket << std::endl;
            if (setNonBlocking(new_socket))
                continue; //Nueva iteracion del bucle
            pollfd client_fd;
            client_fd.fd = new_socket; // Asignar el nuevo socket
            client_fd.events = POLLIN | POLLOUT; // Esperar eventos de entrada y salida
            fds.push_back(client_fd); //Añadirlo al vector de fds (clientes)
        }

        for (size_t i = 1; i < fds.size(); ++i) 
        {
            if (fds[i].revents & POLLIN) 
            {
                // Limpiar el buffer antes de recibir nuevos datos
                std::fill(buffer.begin(), buffer.end(), '\0');
                int bytesRead = recv(fds[i].fd, &buffer[0], buffer.size(), 0);
                if (bytesRead <= 0)
                {
                    handle_error(fds[i].fd, "closed client connection");
                    fds.erase(fds.begin() + i); // Eliminar el socket de la lista
                    --i; // Decrementar 'i' para no omitir el siguiente cliente
                }
                else
                {
                    parseHttpRequest(buffer, fds[i].fd);
                }
            }
        }
    }
    return 0;
}
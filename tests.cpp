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
#include "HttpRequest.hpp"


#define MAX_CLIENTS 100  //Preguntar para que sirve esto, que es un cliente.
#define BUFFER_SIZE 1024 //¿Qué datos?

int handle_error(int fd, std::string a)
{
	std::cerr << "Error en '" << a << "': " << strerror(errno) << std::endl;
	close(fd);
	return -1;
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

void methodGet(HttpRequest &request)
{
	std::string filePath = "./html" + request.path;

	if (request.path.find("..") != std::string::npos)
	{
		std::cerr << "Intento de acceso no autorizado: " << request.path << std::endl;
        request.setResponse("403 Forbidden", 
            "<html><body><h1>403 Forbidden</h1><p>Acceso denegado.</p></body></html>", 
            "text/html");
		return;
	}
	
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
			std::cerr << "Error crítico: No se pudo abrir 404.html tampoco." << std::endl;
			request.setResponse("500 Internal Server Error", 
                "<html><body><h1>500 Internal Server Error</h1><p>No se pudo procesar la solicitud.</p></body></html>", 
                "text/html");
			return;
		}
		request.status = "404 Not Found";
	}
	else
	{
		request.status = "200 OK";
	}
	// Lee el contenido del archivo en un string
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    request.setResponse(request.status, buffer.str(), "text/html");
}

void methodPost(HttpRequest &request)
{
	std::string body = request.body;

	std::string boundary = request.headers["Content-Type"];
	size_t boundaryPos = boundary.find("boundary=");
	if (boundaryPos == std::string::npos)
	{
		request.setResponse("400 Bad Request", 
            "<html><body><h1>Error</h1><p>Formato inválido.</p></body></html>", 
            "text/html");
		return;
	}

	boundary = "--" + boundary.substr(boundaryPos + 9);

	size_t fileStart = body.find("\r\n\r\n", body.find(boundary)) + 4;
	size_t fileEnd = body.find(boundary, fileStart) - 4;

	if (fileStart == std::string::npos || fileEnd == std::string::npos)
	{
		request.setResponse("400 Bad Request", 
            "<html><body><h1>Error</h1><p>No se encontró archivo.</p></body></html>", 
            "text/html");
		return;
	}

	// Extraer el contenido del archivo
	std::string fileContent = body.substr(fileStart, fileEnd - fileStart);

	std::ofstream outFile("./uploads/subido.txt", std::ios::binary);
	if (!outFile.is_open())
	{
		request.setResponse("500 Internal Server Error", 
            "<html><body><h1>Error</h1><p>No se pudo guardar el archivo.</p></body></html>", 
            "text/html");
        return;
	}
	outFile.write(fileContent.c_str(), fileContent.size());
	outFile.close();

	request.setResponse("201 Created", 
        "<html><body><h1>Archivo subido con éxito</h1></body></html>", 
        "text/html");
}


void parseHttpRequest(HttpRequest request)
{

	if (request.method == "GET")
	{
		methodGet(request);
	}
	else if (request.method == "POST")
	{
		methodPost(request);
	}
	else if (request.method == "DELETE")
	{
		request.setResponse("405 Method Not Allowed", 
        "<html><body><h1>Error</h1><p>DELETE no soportado en este servidor estático.</p></body></html", 
        "text/html");
	}
	else
	{
		request.setResponse("501 Not Implemented", 
        "<html><body><h1>Error</h1><p>Método no implementado.</p></body></html>", 
        "text/html");
	}
}


int createServerSocket(int port)
{
	struct sockaddr_in address;
	// Crear socket del servidor
	int server_fd = socket(PF_INET, SOCK_STREAM, 0); //PF_Inet es para IPv4, 
	if (server_fd == -1)
		return handle_error(server_fd, "socket");

	// Configurar socket del servidor para poder ser reusado en on/off rapido
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
		return handle_error(server_fd, "set socket");

	address.sin_family = AF_INET; //Ipv4
	address.sin_addr.s_addr = INADDR_ANY; //Acepta conexiones de cualquier IP
	address.sin_port = htons(port); //Puerto en formato red.

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
		return handle_error(server_fd, "bind");

	if (listen(server_fd, 3) == -1) //3 Conex max en cola. SOMAXCONN if wanted all from oper.system
		return handle_error(server_fd, "listen");

	if (setNonBlocking(server_fd) == -1)
		return -1;  //fallo algó
	
	std::cout << "Servidor escuchando en el puerto " << port << "...\n";
	return server_fd;
}



int main()
{
	int server_fd;
	int new_socket;
	std::string buffer(1024, '\0');
	socklen_t addrlen; // = sizeof(address);
	std::vector<pollfd> fds;
	std::vector<int> ports = {8080, 8081};


	// Bucle para crear los sockets de servidores que escuchan en cada puerto.
	for (int i = 0; i < ports.size(); i++)
	{
		server_fd = createServerSocket(ports[i]);
		if (server_fd == -1)
			return -1;
		pollfd server;
		server.fd = server_fd;
		server.events = POLLIN;  // Monitorear nuevas conexiones
		fds.push_back(server);
	}

	while (true)
	{
		int ret = poll(fds.data(), fds.size(), -1); // Esperar indefinidamente por eventos
		if (ret == -1)
		{
			std::cerr << "Error en poll: " << strerror(errno) << std::endl;
			break; //Salir del bucle si hay error
		}

		for (size_t i = 0; i < fds.size(); ++i) //Empieza en 1 porque el 0 es el servidor
		{
			if (fds[i].revents & POLLIN) 
			{
				if (i < ports.size()) //Sustituir con numero de ports donde lo tengamos
				{
					sockaddr_in client_addr;
					addrlen = sizeof(client_addr);
					new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
					if (new_socket == -1)
					{
						std::cerr << "Error en accept: " << strerror(errno) << std::endl;
						continue; // Continuar con la siguiente iteración del bucle
					}
					std::cout << "Nueva conexión aceptada en el socket " << new_socket << std::endl;
					if (setNonBlocking(new_socket))
						continue; //Nueva iteracion del bucle
					pollfd new_client;
					new_client.fd = new_socket; // Asignar el nuevo socket
					new_client.events = POLLIN | POLLOUT; // Esperar eventos de entrada y salida
					fds.push_back(new_client); //Añadirlo al vector de fds (clientes)
				}
				else
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
						HttpRequest request(buffer);
						parseHttpRequest(request);
						std::string response = request.generateResponse();
						send(fds[i].fd, response.c_str(), response.size(), 0);
					}
				}
				
			}
		}
	}
	return 0;
}
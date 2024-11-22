#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>


class HttpRequest
{
	private:                               // Cuerpo de la solicitud (para POST, PUT, etc.)
		void parseRequestLine(const std::string& requestLine);
		void parseHeaders(std::istream& stream);

	public:
		std::string method;                              // Método HTTP (GET, POST, DELETE)
		std::string path;                                // Ruta (por ejemplo, /upload)
		std::string version;                             // Versión del protocolo HTTP
		std::string queryString;                         // Parámetros de consulta (si existen)
		std::map<std::string, std::string> headers;      // Encabezados HTTP
		std::string body; 
		
		std::string status;                              // Estado de la respuesta (200 OK, 404 Not Found)
		std::map<std::string, std::string> responseHeaders; // Encabezados de la respuesta
		std::string responseBody;                        // Cuerpo de la respuesta
		// Constructor: toma una solicitud HTTP completa como entrada
		explicit HttpRequest(const std::string& rawRequest);

		// Métodos auxiliares

		void setResponse(const std::string& newStatus, const std::string& content, const std::string& contentType);
		std::string generateResponse() const;
		bool isMethodAllowed(const std::vector<std::string>& allowedMethods) const {
			return std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
		}
};

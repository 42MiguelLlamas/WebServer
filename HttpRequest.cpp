#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string& rawRequest)
{
	std::istringstream stream(rawRequest);

	// Parsear la línea de solicitud
	std::string requestLine;
	std::getline(stream, requestLine);
	parseRequestLine(requestLine);

	// Parsear los encabezados
	parseHeaders(stream);

	// Leer el cuerpo, si existe
	if (headers.find("Content-Length") != headers.end())
	{
		int contentLength = std::stoi(headers["Content-Length"]);
		body.resize(contentLength);
		stream.read(&body[0], contentLength);
	}
	// Inicializar atributos de la respuesta por defecto
	status = "200 OK"; // Estado predeterminado
	responseHeaders["Content-Type"] = "text/html";
}


void HttpRequest::parseHeaders(std::istream& stream)
{
	std::string headerLine;
	while (std::getline(stream, headerLine) && headerLine != "\r") {
		size_t separator = headerLine.find(':');
		if (separator != std::string::npos) {
			std::string key = headerLine.substr(0, separator);
			std::string value = headerLine.substr(separator + 1);
			headers[key] = value;
		}
	}
}

void HttpRequest::parseRequestLine(const std::string& requestLine)
{
	std::istringstream lineStream(requestLine);
	lineStream >> method >> path >> version;

	// Extraer parámetros de consulta (si existen)
	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos) {
		queryString = path.substr(queryPos + 1);
		path = path.substr(0, queryPos);
	}
}

void HttpRequest::setResponse(const std::string& newStatus, const std::string& content, const std::string& contentType)
{
	status = newStatus;
	responseBody = content;
	responseHeaders["Content-Type"] = contentType;
	responseHeaders["Content-Length"] = std::to_string(content.size());
}

std::string HttpRequest::generateResponse() const
{
    std::ostringstream responseStream;

    responseStream << version << " " << status << "\r\n";
    // Agregar los encabezados HTTP
    for (std::map<std::string, std::string>::const_iterator it = responseHeaders.begin(); it != responseHeaders.end(); ++it)
	{
		responseStream << it->first << ": " << it->second << "\r\n";
	}
    responseStream << "\r\n";
    responseStream << responseBody;

    return responseStream.str();
}

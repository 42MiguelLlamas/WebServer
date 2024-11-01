#include "Server.hpp"

Server::Server(void)
{
    this->port = 0;
	this->host = 0;
	this->serv_name = "";
	this->root = "";
	this->client_max_body_size = MAX_REQUEST_LENGTH;
	this->index = "";
	this->listen_fd = 0;
	this->autoindex = false;
	this->initErrorWebs();
}

Server::Server(Server const &copy)
{
    this->port = copy.port;
	this->host = copy.host;
	this->serv_name = copy.serv_name;
	this->root = copy.root;
	this->client_max_body_size = copy.client_max_body_size;
	this->index = copy.index;
	this->listen_fd = copy.listen_fd;
	this->autoindex = copy.autoindex;
    this->errorWebs = copy.errorWebs;
    this->locations = copy.locations;
    this->server_address = copy.server_address;
}

Server::~Server()
{
}

/****************************PUBLIC METHODS*****************************/

void	Server::initErrorWebs()
{
	this->errorWebs.insert(std::make_pair(301, "")); //moved permanently
	this->errorWebs.insert(std::make_pair(302, "")); //Found(moved temporarily)
	this->errorWebs.insert(std::make_pair(303, "")); //See other
	this->errorWebs.insert(std::make_pair(307, "")); //Temporary Redirect
	this->errorWebs.insert(std::make_pair(308, "")); //Permanent redirect
	this->errorWebs.insert(std::make_pair(400, "")); //Bad request
	this->errorWebs.insert(std::make_pair(401, "")); //Unauthorized
	this->errorWebs.insert(std::make_pair(402, "")); //Payment required
	this->errorWebs.insert(std::make_pair(403, "")); //Foridden
	this->errorWebs.insert(std::make_pair(404, "")); //Not found
	this->errorWebs.insert(std::make_pair(405, "")); //Method not allowed
	this->errorWebs.insert(std::make_pair(406, "")); //Not acceptable
	this->errorWebs.insert(std::make_pair(408, "")); //Request timeout
	this->errorWebs.insert(std::make_pair(429, "")); //Too Many Request
	this->errorWebs.insert(std::make_pair(500, "")); //Internal server error
	this->errorWebs.insert(std::make_pair(501, "")); //Not implemented
	this->errorWebs.insert(std::make_pair(502, "")); //Bad Gateway
	this->errorWebs.insert(std::make_pair(503, "")); //Service unaviable
	this->errorWebs.insert(std::make_pair(504, "")); //Gateway timeout
	this->errorWebs.insert(std::make_pair(505, "")); //HTTP version not supported
}

void	Server::tryLocation(Location &location) const
{
}

bool	Server::isValidHost(std::string hostName) const //check if host is valid
{
	struct sockaddr_in sockaddr;
	if (inet_pton(AF_INET, hostName.c_str(), &(sockaddr.sin_addr)))
		return true;
	return false;
}

void	Server::endWithSemicolon(std::string &token)//comprueba que acaba en ; y lo elimina
{
	size_t	pos = token.find(';');
	if (pos == std::string::npos)
		throw Config::ConfFileException("Token \"" + token + "\" must end with ;");

	for (int i = pos + 1; i < token.size(); i++)
	{
		if (!isspace(token[i]))
			throw Config::ConfFileException("Wrong token \"" + token + "\"");
	}
	token.erase(pos);
}

bool	Server::checkLocations() const
{
}

int		Server::typeOfPath(std::string &path)
{
	struct stat	buffer;
	int			result;
	
	result = stat(path.c_str(), &buffer);
	if (result == 0)
	{
		if (buffer.st_mode & S_IFREG) //archivo
			return 1;
		else if (buffer.st_mode & S_IFDIR) //directorio
			return 2;
		else
			return 3;
	}
	else
		return -1;
}

/****************************SETTERS*****************************/

void	Server::setServerName(std::string &server_name)
{
	endWithSemicolon(server_name);
	this->serv_name = server_name;
}

void	Server::setHost(std::string &host)
{
	endWithSemicolon(host);
	if (host == "localhost")
		host = "127.0.0.1";
	if (!isValidHost(host))
		throw Config::ConfFileException("Invalid host \"" + host + "\"");
	this->host = inet_addr(host.data());
}

void	Server::setRoot(std::string &root)
{
	endWithSemicolon(root);

	if (typeOfPath(root) == 2)
		this->root = root;
	else
	{
		char dir[1024];
		getcwd(dir, 1024);
		std::string finalRoot = dir + root;
		if (typeOfPath(finalRoot) != 2)
			throw Config::ConfFileException("Invalid root \"" + root + "\"");
		this->root = finalRoot;
	}
}

void	Server::setFd(int &fd)
{
	this->listen_fd = fd;
}

void	Server::setPort(std::string &port)
{
	endWithSemicolon(port);
	
	for (int i=0; i < port.length(); i++)
	{
		if (!isdigit(port[i]))
			throw Config::ConfFileException("Invalid port: \"" + port + "\"");
	}
	int	finalPort = stoi(port);
	if (finalPort < 1 || finalPort > 65636)
		throw Config::ConfFileException("Invalid port: \"" + port + "\"");
	this->port = (uint16_t)finalPort;
}

void	Server::setClientMaxBodySize(std::string &maxBodySize)
{

}

void	Server::setErrorPages(std::vector<std::string> &error_webs)
{

}

void	Server::setIndex(std::string &index)
{

}

void	Server::setLocation(std::string &name, std::vector<std::string> &args)
{

}

void	Server::setAutoindex(std::string &autoindex)
{

}

/****************************GETTERS*****************************/
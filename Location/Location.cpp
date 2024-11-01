#include "Location.hpp"

Location::Location()
{
	this->path = "";
	this->root = "";
	this->index = "";
	this->autoindex = false;
	this->returnPath = "";
	this->alias = "";
	this->client_max_body_size = MAX_REQUEST_LENGTH;
}

Location::~Location()
{
}

Location::Location(Location const &copy)
{
	this->path = copy.path;
	this->root = copy.root;
	this->index = copy.index;
	this->autoindex = copy.autoindex;
	this->returnPath = copy.returnPath;
	this->alias = copy.alias;
	this->client_max_body_size = copy.client_max_body_size;
	this->methods = copy.methods;
	this->cgi_path = copy.cgi_path;
	this->cgi_ext = copy.cgi_ext;
}

/********************************SETTERS***********************************/

void	Location::setPath(std::string const &path)
{
	this->path = path;
}

void	Location::setRootLocation(std::string const &root)
{
	this->root  = root;
}

void	Location::setMethods(std::vector<std::string> const &vecMethods)
{
	this->methods.clear();

	for (int i = 0; i < vecMethods.size(); i++)
	{
		if (vecMethods[i] == "GET")
			this->methods.push_back("GET");
		else if (vecMethods[i] == "POST")
			this->methods.push_back("POST");
		else if (vecMethods[i] == "DELETE")
			this->methods.push_back("DELETE");
		else if (vecMethods[i] == "PUT")
			this->methods.push_back("PUT");
		else if (vecMethods[i] == "HEAD")
			this->methods.push_back("HEAD");
		else
			throw Config::ConfFileException("Method not supported \"" + vecMethods[i] + "\"");
	}
}

void	Location::setAutoindex(std::string const &autoI)
{
	if (autoI == "on")
		this->autoindex = true;
	else if (autoI == "off")
		this->autoindex = false;
	else
		throw Config::ConfFileException("Wrong parameter in autoindex \"" + autoI + "\"");
}

void	Location::setIndexPath(std::string const &indexPath)
{
	this->index = indexPath;
}

void	Location::setReturn(std::string const &retPath)
{
	this->returnPath = retPath;
}

void	Location::setAlias(std::string const &alias)
{
	this->alias = alias;
}

void	Location::setCgiPath(std::vector<std::string> const &cgiPath)
{
	this->cgi_path = cgi_path;
}

void	Location::setCgiExtension(std::vector<std::string> const &cgiExtension)
{
	this->cgi_ext = cgiExtension;
}

void	Location::setMaxBodySize(std::string const &maxBodySize)
{
	unsigned long	aux;

	for (size_t i=0; i < maxBodySize.length(); i++)
	{
		if (!isdigit(maxBodySize[i]))
			throw Config::ConfFileException("Wrong parameter in client_max_body_size: \"" + maxBodySize + "\"");
	}
	this->client_max_body_size = std::stol(maxBodySize);
	if (!client_max_body_size)
		throw Config::ConfFileException("Wrong parameter in client_max_body_size: \"" + maxBodySize + "\"");
}
void	Location::setMaxBodySize(unsigned long const &maxBodySize)
{
	this->client_max_body_size = maxBodySize;
}

/********************************GETTERS***********************************/

const std::string &Location::getPath() const
{
	return this->path;
}

const std::string &Location::getRootLocation() const
{
	return this->root;
}

const std::vector<std::string> &Location::getMethods() const
{
	return this->methods;
}

const bool &Location::getAutoindex() const
{
	return this->autoindex;
}

const std::string &Location::getIndexLocation() const
{
	return this->index;
}

const std::string &Location::getReturn() const
{
	return this->returnPath;
}

const std::string &Location::getAlias() const
{
	return this->alias;
}

const std::vector<std::string> &Location::getCgiPath() const
{
	return this->cgi_path;
}

const std::vector<std::string> &Location::getCgiExtension() const
{
	return this->cgi_ext;
}

const unsigned long &Location::getMaxBodySize() const
{
	return this->client_max_body_size;
}

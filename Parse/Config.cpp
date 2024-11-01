#include "Config.hpp"

//constructor inicial
Config::Config(std::string const &configPath): nb_servers(0)
{
	std::string content = getConfFile(configPath);
	if (content.empty())
		throw Config::ConfFileException("Empty file");

	removeComments(content);
	saveConfigs(content);
	if (this->servers_config.size() == 0 || this->nb_servers == 0)
		throw ConfFileException("No servers in file [" + configPath + "]");
}

//constructor de copia
Config::Config(Config const &copy)
{
	this->nb_servers = copy.nb_servers;
	for (std::vector<std::string>::const_iterator it = copy.servers_config.begin(); it != copy.servers_config.end(); ++it)
		this->servers_config.push_back(*it);
}

// devuelve el contenido de defaultPath en string
std::string	Config::getConfFile(std::string const &defaultPath)
{
	if (defaultPath.empty() || defaultPath.length() == 0)
		throw Config::ConfFileException("Not existing file");
	
	std::ifstream configfd(defaultPath.c_str());
	if (!configfd || !configfd.is_open())
		throw Config::ConfFileException("Not existing file");

	std::string content, line;
	while (std::getline(configfd, line))
		content += line + '\n';
	return content;
}

//elimina los comentarios del archivo de configuracion
void	Config::removeComments(std::string &content)
{
	size_t	pos = content.find('#');
	while (pos != std::string::npos)
	{
		size_t	final_pos = content.find('\n', pos);
		content.erase(pos, final_pos - pos);
		pos = content.find('#');
	}
}

//Guarda todas las configuraciones de servidores del archivo de configuración pasado como argumento
void	Config::saveConfigs(std::string &configContent)
{
	size_t  start;

	for(start = 0; configContent[start]; start++)//quita espacios del principio
	{
		if (!isspace(configContent[start]))
		{
			configContent = configContent.substr(start);
			break ;
		}
	}
	if (configContent.find("server", 0) == std::string::npos)
		throw Config::ConfFileException("Did not find \"server\" in configuration file");
	start = 0;
	size_t  end = 1;
	while (start != end && start < configContent.length())
	{
		start = findStartPos(start, configContent);
		if (start == std::string::npos)
			break ;
		end = findEndPos(start, configContent);
		if (end == std::string::npos)
			throw ConfFileException("Problem with scope{}");
		this->servers_config.push_back(configContent.substr(start, end - start + 1));
		this->nb_servers++;
		start = end + 1;
	}
}

//Encuentra la posicion inicial de { despues de "servidor"
size_t	Config::findStartPos(size_t start, std::string &configContent)
{
	size_t	i;

	for (i = start; configContent[i]; i++)
	{
		if (configContent[i] == 's')
			break ;
		if (!isspace(configContent[i]))
			throw Config::ConfFileException("Wrong character out of server scope{}");
	}
	if (!configContent[i])
		return std::string::npos;
	if (configContent.compare(i, 6, "server") != 0)
		throw Config::ConfFileException("Wrong character out of server scope{}");
	i += 6;
	while (configContent[i] && isspace(configContent[i]))
		i++;
	if (configContent[i] == '{')
		return i;
	else
		throw Config::ConfFileException("Wrong character out of server scope{}");
}

//Encuentra la posicion final de } despues de la posición start
size_t	Config::findEndPos(size_t start, std::string &configContent)
{
	size_t	i;
	size_t	scope;

	scope = 0;
	for (i = start + 1; configContent[i]; i++)
	{
		if (configContent[i] == '{')
			scope++;
		if (configContent[i] == '}')
		{
			if (!scope)
				return (i);
			scope--;
		}
	}
	return std::string::npos;
}

//printear el vector de configuraciones de todos los servidores
void Config::printAllServerConfig()
{
	for (std::vector<std::string>::const_iterator it = this->servers_config.begin(); it != servers_config.end(); ++it) {
        std::cout << *it << std::endl;
    }
}

//destructor
Config::~Config()
{
}
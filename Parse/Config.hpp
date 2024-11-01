#pragma once

# include "../webserv.hpp"
// # include "../server/Server.hpp"

class Config
{
	private:
		int	nb_servers;
		std::vector<std::string> servers_config;

	public:
		//constructors & destructors
			Config(std::string const &defaultPath);
			Config(Config const &copy);
			~Config();

		//public methods
			std::string	getConfFile(std::string const &defaultPath);
			void	removeComments(std::string &content);
			void	saveConfigs(std::string &configContent);
			size_t	findStartPos(size_t start, std::string &configContent);
			size_t	findEndPos(size_t start, std::string &configContent);

		//print configs
			void	printAllServerConfig();

		//exceptions
			class ConfFileException: public std::exception
			{
				private:
					std::string message;
				public:
					ConfFileException(std::string const &msg) throw()
					{
						message = "Configuration file error: " + msg;
					}
					virtual const char *what() const throw()
					{
						return (message.c_str());
					}
					virtual ~ConfFileException() throw() {}
			};
};

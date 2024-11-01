#pragma once

# include "../webserv.hpp"
# include "../Parse/Config.hpp"

# define MAX_REQUEST_LENGTH 30000000

class Location //vector de locations en Server
{
	private:
		std::string					path;
		std::string					root;
		std::string					index;
		bool						autoindex;
		std::vector<std::string>	methods;
		std::string					returnPath;
		std::string					alias;
		std::vector<std::string>	cgi_path;
		std::vector<std::string>	cgi_ext;
		unsigned long				client_max_body_size;

	public:
		//constructor & destructor
			Location();
			Location(Location const &copy);
			~Location();

		//setters
			void setPath(std::string const &path);
			void setRootLocation(std::string const &root);
			void setMethods(std::vector<std::string> const &vecMethods);
			void setAutoindex(std::string const &autoI);
			void setIndexPath(std::string const &indexPath);
			void setReturn(std::string const &retPath);
			void setAlias(std::string const &alias);
			void setCgiPath(std::vector<std::string> const &cgiPath);
			void setCgiExtension(std::vector<std::string> const &cgiExtension);
			void setMaxBodySize(std::string const &maxBodySize);
			void setMaxBodySize(unsigned long const &maxBodySize);

		//getters
			const std::string &getPath() const;
			const std::string &getRootLocation() const;
			const std::vector<std::string> &getMethods() const;
			const bool &getAutoindex() const;
			const std::string &getIndexLocation() const;
			const std::string &getReturn() const;
			const std::string &getAlias() const;
			const std::vector<std::string> &getCgiPath() const;
			const std::vector<std::string> &getCgiExtension() const;
			const unsigned long &getMaxBodySize() const;
};
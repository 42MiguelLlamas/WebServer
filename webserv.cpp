#include "webserv.hpp"
#include "Parse/Config.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Error: invalid number of arguments" << std::endl;
		return (1);
	}
	try
	{
		Config conf(argv[1]);
		conf.printAllServerConfig();
    }
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return (0);
}
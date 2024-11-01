#pragma once

# include "../webserv.hpp"

class serverPool
{
	private:
		
	public:
		//consructors & destructors
			serverPool();
			~serverPool();
		
		//public methods
			void	parseConfig(std::string const &fileConfig);
			void	runAll();
};
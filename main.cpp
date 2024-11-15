#include "app.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	wind::App app{};

	try
	{
		app.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
	
}
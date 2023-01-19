#include "Application.h"
#include "Global.hpp"

#include "../Rendering/Shader.h"

#include <iostream>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int Hayase::WindowInfo::windowWidth = 1600;
int Hayase::WindowInfo::windowHeight = 900;
std::string Hayase::Shader::defaultDirectory = "Materials/Shaders";

int main()
{
	Hayase::Application app{ Hayase::WindowInfo::windowWidth, Hayase::WindowInfo::windowHeight };
	
	std::cout << "Hello, Hello World!" << std::endl;
	
	try
	{
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
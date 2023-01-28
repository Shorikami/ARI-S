#include <hyspch.h>

#include "Application.h"

#include "Shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h> 

std::string Hayase::Shader::defaultDirectory = "Materials/Shaders/";


int main()
{
	srand((unsigned)time(NULL));
	Hayase::Application app{ 1600, 900 };
	
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
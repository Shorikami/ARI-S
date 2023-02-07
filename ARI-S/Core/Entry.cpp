#include <arpch.h>

#include "Application.h"

#include "Shader.h"
#include "ModelBuilder.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h> 

std::string ARIS::Shader::defaultDirectory = "Content/Assets/Shaders/";


int main()
{
	srand((unsigned)time(NULL));

	// Application MUST be built first before the model table
	ARIS::Application app{ 1600, 900 };
	ARIS::ModelBuilder mb;
	
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
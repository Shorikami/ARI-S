#include "Texture.h"

#include <iostream>
#include <stb/stb_image.h>

namespace Hayase
{
	Texture::Texture(std::string name)
		: name(name)
	{
		Generate();
	}

	Texture::Texture(std::string dir, std::string path)
		: dir(dir)
		, path(path)
	{
		Generate();
	}

	void Texture::Generate()
	{
		glGenTextures(1, &ID);
	}

	void Texture::Load(bool flip)
	{
		stbi_set_flip_vertically_on_load(flip);

		int width, height, nChannels;
		unsigned char* data = stbi_load((dir + "/" + path).c_str(), &width, &height, &nChannels, 0);

		GLenum colorMode = GL_RGB;

		switch (nChannels)
		{
		case 1:
			colorMode = GL_RED;
			break;
		case 4:
			colorMode = GL_RGBA;
			break;
		}

		if (data)
		{
			glBindTexture(GL_TEXTURE_2D, ID);
			glTexImage2D(GL_TEXTURE_2D, 0, colorMode, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else
		{
			std::cout << "Image not loaded at " << path << std::endl;
		}

		stbi_image_free(data);
	}

	// Empty texture
	void Texture::Allocate(GLenum format, GLuint w, GLuint h, GLenum type)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, type, NULL);
	}

	void Texture::SetParameters(GLenum texMin, GLenum texMag, GLenum wrapS, GLenum wrapT)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texMag);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texMin);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
	}

	void Texture::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, ID);
	}

	void Texture::Bind(GLuint slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, ID);
	}

	void Texture::Cleanup()
	{
		glDeleteTextures(1, &ID);
	}
}
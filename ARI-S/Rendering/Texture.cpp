#include <arpch.h>

#include "Texture.h"

#include <stb_image.h>

namespace ARIS
{
	Texture::Texture()
		: m_ID(0)
		, m_Width(0)
		, m_Height(0)
		, m_InternalFormat(0)
		, m_DataFormat(0)
		, m_IsLoaded(false)
		, name(std::string())
	{
		Generate();
	}

	Texture::Texture(std::string name_)
		: m_ID(0)
		, m_Width(0)
		, m_Height(0)
		, m_InternalFormat(0)
		, m_DataFormat(0)
		, m_IsLoaded(false)
		, name(name_)
	{
		Generate();
	}

	Texture::Texture(std::string dir, std::string path)
		: dir(dir)
		, m_Path(path)
	{
		Generate();
	}

	Texture::Texture(GLuint w, GLuint h, GLenum intForm, GLenum dataForm, void* data, GLenum filter, GLenum repeat, GLenum type)
		: m_Width(w)
		, m_Height(h)
	{
		m_InternalFormat = intForm;
		m_DataFormat = dataForm;

		glGenTextures(1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexImage2D(GL_TEXTURE_2D, 0, intForm, w, h, 0, dataForm, type, NULL);

		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);

		if (repeat == GL_CLAMP_TO_BORDER)
		{
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		}

		if (data != nullptr)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, intForm, w, h, 0, dataForm, type, data);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture::Texture(const std::string& path, GLenum filter, GLenum repeat, bool hdr)
		: m_IsLoaded(false)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);

		if (hdr)
		{
			int test = stbi_is_hdr(path.c_str());
		}
	
		glGenTextures(1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);
	
		
		if (hdr)
		{
			float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);

			if (data != nullptr)
			{
				m_IsLoaded = true;
				m_Width = width;
				m_Height = height;
				m_Path = path;

				GLenum dataFormat = 0;
				if (channels == 4)
				{
					dataFormat = GL_RGBA;
				}
				else if (channels == 3)
				{
					dataFormat = GL_RGB;
				}
				else if (channels == 1)
				{
					dataFormat = GL_RED;
				}

				m_DataFormat = dataFormat;
				m_InternalFormat = channels == 3 ? GL_RGB16F : GL_RGBA16F;

				GLenum loadAs = hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

				//glTextureStorage2D(m_ID, 1, internalFormat, m_Width, m_Height);
				glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, loadAs, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
			{
				std::cout << "Texture failed!" << std::endl;
			}
		}
			
		else
		{
			stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

			if (data != nullptr)
			{
				m_IsLoaded = true;
				m_Width = width;
				m_Height = height;
				m_Path = path;

				GLenum dataFormat = 0;
				if (channels == 4)
				{
					dataFormat = GL_RGBA;
				}
				else if (channels == 3)
				{
					dataFormat = GL_RGB;
				}
				else if (channels == 1)
				{
					dataFormat = GL_RED;
				}

				m_DataFormat = dataFormat;
				m_InternalFormat = channels == 3 ? GL_RGB16F : GL_RGBA16F;

				GLenum loadAs = hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

				//glTextureStorage2D(m_ID, 1, internalFormat, m_Width, m_Height);
				glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, loadAs, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
			{
				std::cout << "Texture failed!" << std::endl;
			}
		}
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, repeat);

		if (repeat == GL_CLAMP_TO_BORDER)
		{
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
	}

	void Texture::Generate()
	{
		glGenTextures(1, &m_ID);
	}

	void Texture::Load(bool flip)
	{
		stbi_set_flip_vertically_on_load(flip);

		int width, height, nChannels;
		unsigned char* data = stbi_load((dir + "/" + m_Path).c_str(), &width, &height, &nChannels, 0);

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
			glBindTexture(GL_TEXTURE_2D, m_ID);
			glTexImage2D(GL_TEXTURE_2D, 0, colorMode, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else
		{
			std::cout << "Image not loaded at " << m_Path << std::endl;
		}

		stbi_image_free(data);
	}

	void Texture::AllocateCubemap(GLuint width, GLuint height, GLenum intForm, GLenum dataForm,
		GLenum filter, GLenum repeat, GLenum type)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		for (unsigned i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, intForm, width, height, 0, dataForm, type, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, repeat);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, repeat);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, repeat);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter);
	}

	void Texture::LoadCubemap(std::vector<std::string> faces)
	{
		stbi_set_flip_vertically_on_load(false);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		int width, height, nrChannels;

		for (unsigned i = 0; i < faces.size(); ++i)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				GLenum format = nrChannels == 4 ? GL_RGBA : (nrChannels == 3 ? GL_RGB : GL_RED);

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			}
			else
			{
				std::cout << "Face failed to load at path: " << faces[i] << std::endl;
			}

			stbi_image_free(data);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	// Empty texture
	void Texture::Allocate(GLenum interForm, GLenum dataForm, GLuint w, GLuint h, GLenum type)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, interForm, w, h, 0, dataForm, type, NULL);
	}

	void Texture::SetParameters(GLenum texMin, GLenum texMag, GLenum wrapS, GLenum wrapT)
	{
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texMag);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texMin);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

		if (wrapS == GL_CLAMP_TO_BORDER && wrapT == GL_CLAMP_TO_BORDER)
		{
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
	}

	void Texture::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	void Texture::Bind(GLuint slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	void Texture::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::Cleanup()
	{
		glDeleteTextures(1, &m_ID);
	}
}
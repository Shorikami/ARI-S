
#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

namespace ARIS
{
	class Texture
	{
	public:
		Texture();
		Texture(std::string name);
		Texture(std::string dir, std::string path);

		Texture(GLuint width, GLuint height, 
			GLenum intForm, GLenum dataForm, void* data = nullptr, 
			GLenum filter = GL_LINEAR, GLenum repeat = GL_REPEAT, GLenum type = GL_UNSIGNED_BYTE);

		Texture(const std::string& path, GLenum filter, GLenum repeat, bool hdr = false);

		void Generate();
		void Load(bool flip = true);
		
		void LoadCubemap(std::vector<std::string> faces);
		void AllocateCubemap(GLuint width, GLuint height, GLenum intForm, GLenum dataForm, 
			GLenum filter, GLenum repeat, GLenum type, bool genMipMaps = false);

		void Allocate(GLenum interForm, GLenum dataForm, GLuint width, GLuint height, GLenum type);

		

		static void SetParameters(GLenum texMin = GL_NEAREST, GLenum texMag = GL_NEAREST,
			GLenum wrapS = GL_REPEAT, GLenum wrapT = GL_REPEAT);

		void Bind();
		void Bind(GLuint slot);

		void Unbind();

		void Cleanup();

		unsigned m_ID;
		//aiTextureType type;
		std::string dir, m_Path, name;

		//GLuint m_ID, m_BindPort;
		GLuint m_Width, m_Height;
		GLenum m_InternalFormat, m_DataFormat;
		GLboolean m_IsLoaded;
	};
}


#endif
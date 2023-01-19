
#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

namespace Hayase
{
	class Texture
	{
	public:
		Texture(std::string name);
		Texture(std::string dir, std::string path);

		void Generate();
		void Load(bool flip = true);
		void Allocate(GLenum format, GLuint width, GLuint height, GLenum type);

		static void SetParameters(GLenum texMin = GL_NEAREST, GLenum texMag = GL_NEAREST,
			GLenum wrapS = GL_REPEAT, GLenum wrapT = GL_REPEAT);

		void Bind();
		void Bind(GLuint slot);

		void Cleanup();

		unsigned ID;
		//aiTextureType type;
		std::string dir, path, name;
	};
}


#endif
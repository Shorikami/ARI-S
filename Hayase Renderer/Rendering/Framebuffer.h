#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <glm.hpp>

#include <vector>

namespace Hayase
{
	class Framebuffer
	{
	public:
		Framebuffer(GLuint id);
		static GLuint CreateFBO(std::vector<GLuint> const& cols, GLuint depth = GL_NONE);

		~Framebuffer();

		void Bind();
		static void Unbind();
		void Clear(GLenum buffer, GLint drawBuffer, const GLfloat* val);
		GLuint GetID() const;
	private:
		GLuint m_ID;
	};
}

#endif
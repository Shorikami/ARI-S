#include "Framebuffer.h"

#include <array>
#include <iostream>

namespace Hayase
{
	Framebuffer::Framebuffer(GLuint id)
		: m_ID(id)
	{
	}

	GLuint Framebuffer::CreateFBO(std::vector<GLuint> const& cols, GLuint depth)
	{
		GLuint fbo = 0;
		glGenFramebuffers(1, &fbo);

		for (unsigned i = 0; i < cols.size(); ++i)
		{
			glFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0 + i, cols[i], 0);
		}

		std::array<GLenum, 32> drawBuffers;

		for (GLenum i = 0; i < cols.size(); ++i)
		{
			drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		}

		// Will this work?
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glDrawBuffers(cols.size(), drawBuffers.data());

		if (depth != GL_NONE)
		{
			glFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, depth, 0);
		}

		if (glCheckFramebufferStatus(fbo) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Uh oh! Framebuffer incomplete!" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return fbo;
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_ID);
	}

	void Framebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
	}

	void Framebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Clear(GLenum buffer, GLint drawBuffer, const GLfloat* val)
	{
		glClearBufferfv(buffer, drawBuffer, const_cast<GLfloat*>(val));
	}

	GLuint Framebuffer::GetID() const
	{
		return m_ID;
	}
}
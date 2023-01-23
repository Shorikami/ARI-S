#include "Framebuffer.h"
#include "../Core/Global.hpp"

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
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		for (unsigned i = 0; i < cols.size(); ++i)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, cols[i], 0);
		}

		std::array<GLenum, 32> drawBuffers;

		for (GLenum i = 0; i < cols.size(); ++i)
		{
			drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		glDrawBuffers(cols.size(), drawBuffers.data());


		// If a depth texture is specified, use that for depth to texture rendering by attaching
		// it as a 2D texture
		if (depth != GL_NONE)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		}

		// Otherwise, just generate a default render buffer and attach it
		else
		{
			unsigned int rbo;
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WindowInfo::windowWidth, WindowInfo::windowHeight);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
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
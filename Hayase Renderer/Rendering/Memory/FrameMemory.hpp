#ifndef FRAMEMEMORY_HPP
#define FRAMEMEMORY_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "../Texture.h"
#include "../Objects/Cubemap.h"

class RenderbufferObject
{
public:
	GLuint val;

	void Generate()
	{
		glGenRenderbuffers(1, &val);
	}

	void Bind()
	{
		glBindRenderbuffer(GL_RENDERBUFFER, val);
	}

	void Allocate(GLenum format, GLuint width, GLuint height)
	{
		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
	}

	void Cleanup()
	{
		glDeleteRenderbuffers(1, &val);
	}
};

class FramebufferObject
{
public:
	GLuint val;
	GLuint width, height;
	GLbitfield bitCombo;

	std::vector<GLuint> rbos;
	std::vector<Texture> textures;
	Cubemap cubemap;

	FramebufferObject()
		: val(0)
		, width(0)
		, height(0)
		, bitCombo(0)
	{
	}

	FramebufferObject(GLuint width, GLuint height, GLbitfield bitCombo)
		: val(0)
		, width(width)
		, height(height)
		, bitCombo(bitCombo)
	{
	}

	void Generate()
	{
		glGenFramebuffers(1, &val);
	}

	void DisableColorBuffer()
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	void Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, val);
	}

	void Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void SetViewport()
	{
		glViewport(0, 0, width, height);
	}

	void Clear()
	{
		glClear(bitCombo);
	}

	void Activate()
	{
		SetViewport();
		Bind();
		Clear();
		//glClear(GL_DEPTH_BUFFER_BIT);
	}

	void AllocateAttachRBO(GLenum type, GLenum format)
	{
		GLuint rbo;

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);

		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, type, GL_RENDERBUFFER, rbo);

		rbos.push_back(rbo);
	}

	void AllocateAttachTexture(GLenum attachType, GLenum format, GLenum type)
	{
		std::string name = "tex" + textures.size();
		Texture tex(name);

		tex.Bind();
		tex.Allocate(format, width, height, type);
		Texture::SetParameters(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glFramebufferTexture2D(GL_FRAMEBUFFER, attachType, GL_TEXTURE_2D, tex.ID, 0);

		textures.push_back(tex);
	}

	void AttachTexture(GLenum type, Texture t)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, t.ID, 0);
	}

	void AllocateAttachCubemap(GLenum attachType, GLenum format, GLenum type)
	{
		cubemap = Cubemap();
		cubemap.Generate();
		cubemap.Bind();
		cubemap.Allocate(format, width, height, type);

		glFramebufferTexture(GL_FRAMEBUFFER, attachType, cubemap.ID, 0);
	}

	void Cleanup()
	{
		glDeleteRenderbuffers(rbos.size(), &rbos[0]);

		for (Texture t : textures)
		{
			t.Cleanup();
		}

		glDeleteFramebuffers(1, &val);
	}
};

#endif
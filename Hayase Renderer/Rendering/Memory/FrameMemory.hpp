#ifndef FRAMEMEMORY_HPP
#define FRAMEMEMORY_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <glm.hpp>
#include <array>

#include "Texture.h"

namespace Hayase
{
	class Renderbuffer
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

	class Framebuffer
	{
	public:
		Framebuffer()
			: m_ID(0)
			, m_Width(0)
			, m_Height(0)
			, m_BitCombo(0)
		{
			Refresh();
		}

		Framebuffer(GLuint width, GLuint height, GLbitfield bitCombo)
			: m_ID(0)
			, m_Width(width)
			, m_Height(height)
			, m_BitCombo(bitCombo)
		{
			Refresh();
		}

		void Generate()
		{
			glGenFramebuffers(1, &m_ID);
		}

		void DisableColorBuffer()
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		void Bind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
		}

		void Unbind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void SetViewport()
		{
			glViewport(0, 0, m_Width, m_Height);
		}

		void Clear()
		{
			glClear(m_BitCombo);
		}

		void Activate()
		{
			SetViewport();
			Bind();
			Clear();
		}

		void Refresh()
		{
			std::vector<Texture> colorCopy = m_ColorAttachments;
			Texture depthCopy = m_DepthAttachment;

			if (m_ID)
			{
				Cleanup();

				m_ColorAttachments.clear();
				m_DepthAttachment.ID = 0;
			}

			Generate();
			Bind();

			for (size_t i = 0; i < colorCopy.size(); ++i)
			{
				AttachTexture(GL_COLOR_ATTACHMENT0 + i, colorCopy[i]);
			}

			if (m_ColorAttachments.size() > 0)
			{
				DrawBuffers();
			}
			
			if (depthCopy.ID != 0)
			{
				AttachTexture(GL_DEPTH_ATTACHMENT, depthCopy);
			}
			

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cout << "Uh oh! Refresh() command is incomplete!" << std::endl;
			}

			Unbind();
		}

		void Resize(glm::vec2 size)
		{
			m_Width = size.x;
			m_Height = size.y;

			Refresh();
		}

		void Resize(uint32_t width, uint32_t height)
		{
			m_Width = width;
			m_Height = height;

			Refresh();
		}

		void AllocateAttachRBO(GLenum type, GLenum format)
		{
			GLuint rbo;

			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);

			glRenderbufferStorage(GL_RENDERBUFFER, format, m_Width, m_Height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, type, GL_RENDERBUFFER, rbo);

			m_RBOs.push_back(rbo);
		}

		void AllocateAttachTexture(GLenum attachType, GLenum format, GLenum type)
		{

			std::string name = "tex" + m_ColorAttachments.size();
			Texture tex(name);

			tex.Bind();
			tex.Allocate(format, m_Width, m_Height, type);
			Texture::SetParameters(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachType, GL_TEXTURE_2D, tex.ID, 0);

			if (attachType == GL_DEPTH_ATTACHMENT)
			{
				m_DepthAttachment = tex;
			}
			else
			{
				m_ColorAttachments.push_back(tex);
			}
		}

		void AttachTexture(GLenum type, Texture t)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, t.ID, 0);
			if (type == GL_DEPTH_ATTACHMENT)
			{
				m_DepthAttachment = t;
			}
			else
			{
				m_ColorAttachments.push_back(t);
			}
		}

		void DrawBuffers()
		{
			std::array<GLenum, 32> drawBuffers;

			for (unsigned i = 0; i < m_ColorAttachments.size(); ++i)
			{
				drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
			}
			
			glDrawBuffers(m_ColorAttachments.size(), drawBuffers.data());
		}

		void Cleanup()
		{
			glDeleteRenderbuffers(m_RBOs.size(), &m_RBOs[0]);

			for (Texture t : m_ColorAttachments)
			{
				t.Cleanup();
			}

			m_DepthAttachment.Cleanup();

			glDeleteFramebuffers(1, &m_ID);
		}

		GLuint GetID() const { return m_ID; }
		GLuint GetWidth() const { return m_Width; }
		GLuint GetHeight() const { return m_Height; }
		GLbitfield GetBitCombo() const { return m_BitCombo; }
		std::vector<GLuint> GetRBOs() const { return m_RBOs; }
		std::vector<Texture> GetColorAttachments() const { return m_ColorAttachments; }
		
		Texture GetColorAttachment(int idx) const
		{
			if (idx >= m_ColorAttachments.size())
			{
				return m_ColorAttachments[0];
			}
			return m_ColorAttachments[idx];
		}
		Texture GetDepthAttachment() const { return m_DepthAttachment; }

	private:
		GLuint m_ID;
		GLuint m_Width, m_Height;
		GLbitfield m_BitCombo;

		std::vector<GLuint> m_RBOs;
		std::vector<Texture> m_ColorAttachments;
		Texture m_DepthAttachment;

	};
}

#endif
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

	struct FramebufferTexture
	{
		FramebufferTexture() = default;
		FramebufferTexture(GLenum att, GLenum form, GLenum type)
			: s_AttachType(att)
			, s_Format(form)
			, s_Type(type)
		{
		}

		GLenum s_AttachType = 0;
		GLenum s_Format = 0;
		GLenum s_Type = 0;
	};

	struct FramebufferAttachments
	{
		FramebufferAttachments() = default;
		FramebufferAttachments(std::initializer_list<FramebufferTexture> attachments)
			: s_Attachments(attachments)
		{
		}

		std::vector<FramebufferTexture> s_Attachments;
		uint32_t s_DepthFormat = 0;
	};

	struct FramebufferSpecs
	{
		FramebufferSpecs() = default;

		uint32_t s_Width = 0, s_Height = 0;
		GLbitfield s_BitCombo = 0;

		FramebufferAttachments s_Attachments;
	};

	class Framebuffer
	{
	public:
		Framebuffer()
			: m_ID(0)
			, m_Specs()
		{
			Generate();
		}

		Framebuffer(GLuint width, GLuint height, GLbitfield bitCombo)
			: m_ID(0)
		{
			m_Specs.s_Width = width;
			m_Specs.s_Height = height;
			m_Specs.s_BitCombo = bitCombo;

			Generate();
		}

		~Framebuffer()
		{
			Cleanup();
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
			glViewport(0, 0, m_Specs.s_Width, m_Specs.s_Height);
		}

		void Clear()
		{
			glClear(m_Specs.s_BitCombo);
		}

		void Activate()
		{
			Bind();
			SetViewport();
			Clear();
		}

		// I think this causes memory leaks lol
		void Refresh()
		{
			if (m_ID)
			{
				Cleanup();

				m_ColorAttachments.clear();
				m_DepthAttachment.m_ID = 0;
			}

			Generate();
			Bind();

			for (size_t i = 0; i < m_Specs.s_Attachments.s_Attachments.size(); ++i)
			{
				AllocateAttachTexture(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), GL_RGBA, GL_UNSIGNED_BYTE, true);
			}

			if (m_ColorAttachments.size() > 0)
			{
				DrawBuffers();
			}
			
			if (m_Specs.s_Attachments.s_DepthFormat != 0)
			{
				AllocateAttachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_FLOAT, true);
			}

			int res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (res != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cout << "Uh oh! Refresh() command is incomplete!" << std::endl;
			}

			Unbind();
		}

		void Resize(glm::vec2 size)
		{
			m_Specs.s_Width = static_cast<uint32_t>(size.x);
			m_Specs.s_Height = static_cast<uint32_t>(size.y);

			Refresh();
		}

		void Resize(uint32_t width, uint32_t height)
		{
			m_Specs.s_Width = width;
			m_Specs.s_Height = height;

			Refresh();
		}

		//void AllocateAttachRBO(GLenum type, GLenum format)
		//{
		//	GLuint rbo;
		//
		//	glGenRenderbuffers(1, &rbo);
		//	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		//
		//	glRenderbufferStorage(GL_RENDERBUFFER, format, m_Width, m_Height);
		//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, type, GL_RENDERBUFFER, rbo);
		//
		//	m_RBOs.push_back(rbo);
		//}

		void AllocateAttachTexture(GLenum attachType, GLenum format, GLenum type, bool reallocate = false)
		{
			std::string name = "texture" + std::to_string(m_ColorAttachments.size());
			Texture tex(name);
			tex.m_Width = m_Specs.s_Width;
			tex.m_Height = m_Specs.s_Height;
			tex.m_DataFormat = tex.m_InternalFormat = format;

			tex.Bind();
			tex.Allocate(format, m_Specs.s_Width, m_Specs.s_Height, type);
			Texture::SetParameters(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

			//float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachType, GL_TEXTURE_2D, tex.m_ID, 0);

			if (attachType == GL_DEPTH_ATTACHMENT)
			{
				m_DepthAttachment = tex;
				
				if (!reallocate)
				{
					m_Specs.s_Attachments.s_DepthFormat = format;
				}
			}
			else
			{
				m_ColorAttachments.push_back(tex);

				if (!reallocate)
				{
					m_Specs.s_Attachments.s_Attachments.push_back(FramebufferTexture(attachType, format, type));
				}
			}
		}

		void AttachTexture(GLenum type, Texture t)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, t.m_ID, 0);
			if (type == GL_DEPTH_ATTACHMENT)
			{
				m_DepthAttachment = t;
				m_Specs.s_Attachments.s_DepthFormat = GL_DEPTH_COMPONENT;
			}
			else
			{
				m_ColorAttachments.push_back(t);
				m_Specs.s_Attachments.s_Attachments.push_back(FramebufferTexture(type, GL_RGBA, GL_UNSIGNED_BYTE));
			}
		}

		void DrawBuffers()
		{
			std::array<GLenum, 32> drawBuffers;

			for (unsigned i = 0; i < m_ColorAttachments.size(); ++i)
			{
				drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
			}
			
			glDrawBuffers(static_cast<GLsizei>(m_ColorAttachments.size()), drawBuffers.data());
		}

		void Cleanup()
		{
			for (Texture t : m_ColorAttachments)
			{
				t.Cleanup();
			}

			m_DepthAttachment.Cleanup();

			glDeleteFramebuffers(1, &m_ID);
		}

		GLuint GetID() const { return m_ID; }
		FramebufferSpecs GetSpecs() const { return m_Specs; }
		void SetWidth(uint32_t w) { m_Specs.s_Width = w; }
		void SetHeight(uint32_t h) { m_Specs.s_Height = h; }
		
		Texture GetColorAttachment(int idx = 0) const
		{
			assert(idx < m_ColorAttachments.size() && "You tried to get a color attachment out of bounds!");
			return m_ColorAttachments[idx];
		}

		Texture GetDepthAttachment() { return m_DepthAttachment; }

	private:
		GLuint m_ID;
		FramebufferSpecs m_Specs;

		std::vector<Texture> m_ColorAttachments;
		Texture m_DepthAttachment;
	};
}

#endif
#include <hyspch.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window.h"
#include "AppEvent.h"
#include "KeyEvent.h"
#include "MouseEvent.h"

namespace Hayase
{
	static unsigned s_WindowCount = 0;

	WindowBase* WindowBase::Generate(const WindowProperties& info)
	{
		return new Window(info);
	}

	Window::Window(const WindowProperties& info)
	{
		Init(info);
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Init(const WindowProperties& info)
	{
		m_Data.s_Width = info.s_Width;
		m_Data.s_Height = info.s_Height;
		m_Data.s_Title = info.s_Title;

		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW!");
		}
		
		glfwWindowHint(GLFW_SAMPLES, 1); // change for anti-aliasing
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, info.s_MajorVer);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, info.s_MinorVer);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(info.s_Width, info.s_Height, m_Data.s_Title.c_str(), nullptr, nullptr);
		

		if (m_Window == nullptr)
		{
			glfwTerminate();
			throw std::runtime_error(std::string("Failed to create window! Is your GPU compatible with " 
				+ std::to_string(info.s_MajorVer) + "." + std::to_string(info.s_MinorVer) + "?").c_str());
		}

		++s_WindowCount;

		glfwMakeContextCurrent(m_Window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			glfwTerminate();
			throw std::runtime_error("Failed to initialize GLAD!");
		}

		glfwSetWindowUserPointer(m_Window, &m_Data);

		SetVSync(1);

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int code, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		
			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key);
					data.s_EventCB(event);
					break;
				}
		
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.s_EventCB(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyPressedEvent event(key);
					data.s_EventCB(event);
					break;
				}
			}
		});
		
		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned code)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		
				KeyTypedEvent event(code);
				data.s_EventCB(event);
			});
		
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		
				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data.s_EventCB(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data.s_EventCB(event);
						break;
					}
				}
			});
		
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		
				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.s_EventCB(event);
			});
		
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		
				MouseMovedEvent event((float)xPos, (float)yPos);
				data.s_EventCB(event);
			});
		
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.s_Width = width;
				data.s_Height = height;
		
				WindowResizeEvent event(width, height);
				data.s_EventCB(event);
			});
		
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.s_EventCB(event);
			});
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		--s_WindowCount;

		if (s_WindowCount == 0)
		{
			glfwTerminate();
		}
	}

	void Window::Update()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

	void Window::SetVSync(bool b)
	{
		glfwSwapInterval(b ? 1 : 0);
		m_Data.s_VSync = b;
	}

	bool Window::IsVSync() const
	{
		return m_Data.s_VSync;
	}
}
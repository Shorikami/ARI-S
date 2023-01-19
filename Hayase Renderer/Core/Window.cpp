#include "Window.h"

#include "../IO/Mouse.h"
#include "../IO/Keyboard.h"

// Include ImGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <stdexcept>

namespace Hayase
{

	Window::Window(int w, int h, std::string name, int glfwVerMajor, int glfwVerMinor)
		: width(w)
		, height(h)
		, windowName(name)
	{
		InitWindow(glfwVerMajor, glfwVerMinor);
	}

	Window::~Window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::InitWindow(int major, int minor)
	{
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW!");
		}
		
		glfwWindowHint(GLFW_SAMPLES, 1); // change for anti-aliasing
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		if (window == nullptr)
		{
			glfwTerminate();
			throw std::runtime_error(std::string("Failed to create window! Is your GPU compatible with " 
				+ std::to_string(major) + "." + std::to_string(minor) + "?").c_str());
		}

		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, this);

		glfwSetKeyCallback(window, Keyboard::KeyCallback);
		glfwSetCursorPosCallback(window, Mouse::CursorPosCallback);
		glfwSetScrollCallback(window, Mouse::MouseWheelCallback);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			glfwTerminate();
			throw std::runtime_error("Failed to initialize GLAD!");
		}

		// Ensure we can capture the escape key being pressed below
		glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

		// Initialize ImGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);

		std::string version = std::string("#version ") + std::to_string(major) + std::to_string(minor) + std::string("0");

		ImGui_ImplOpenGL3_Init(version.c_str());
	}

	void Window::FramebufferResizeCallback(GLFWwindow* window, int w, int h)
	{
		auto wWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		wWindow->frameBufferResized = true;
		wWindow->width = w;
		wWindow->height = h;
	}
}
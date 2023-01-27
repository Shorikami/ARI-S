#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <GLFW/glfw3.h>
#include <string>

#include <sstream>
#include <functional>

#include "../Events/Event.h"

namespace Hayase
{
	struct WindowProperties
	{
		unsigned s_Width, s_Height;
		unsigned s_MajorVer, s_MinorVer;
		std::string s_Title;

		WindowProperties(unsigned w = 1280, unsigned h = 720, unsigned major = 4, unsigned minor = 3, const std::string& t = "Hayase Renderer")
			: s_Width(w)
			, s_Height(h)
			, s_MajorVer(major)
			, s_MinorVer(minor)
			, s_Title(t)
		{
		}
	};

	class WindowBase
	{
	public:
		using EventCallbackFunc = std::function<void(Event&)>;

		virtual ~WindowBase() = default;

		virtual void Update() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFunc& cb) = 0;
		virtual void SetVSync(bool b) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetWindow() const = 0;
		
		static WindowBase* Generate(const WindowProperties& props = WindowProperties());
	};

	class Window : public WindowBase
	{
	public:
		Window(const WindowProperties& info);
		virtual ~Window();

		void Update() override;

		unsigned GetWidth() const override { return m_Data.s_Width; }
		unsigned GetHeight() const override { return m_Data.s_Height; }

		void SetEventCallback(const EventCallbackFunc& cb) override { m_Data.s_EventCB = cb; }
		void SetVSync(bool b) override;
		bool IsVSync() const override;

		virtual void* GetWindow() const { return m_Window; }

	private:
		virtual void Init(const WindowProperties& info);
		virtual void Shutdown();

		GLFWwindow* m_Window;

		struct WindowData
		{
			std::string s_Title;
			unsigned s_Width, s_Height;
			bool s_VSync;

			EventCallbackFunc s_EventCB;
		};

		WindowData m_Data;
	};
}

#endif
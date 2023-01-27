#ifndef APPEVENT_H
#define APPEVENT_H

#include "Event.h"

namespace Hayase
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned w, unsigned h)
			: m_Width(w)
			, m_Height(h)
		{
		}

		unsigned GetWidth() const { return m_Width; }
		unsigned GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			std::stringstream s;
			s << "WindowResizeEvent: " << m_Width << ", " << m_Height;
			return s.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(Application)

	private:
		unsigned m_Width, m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(Application)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		EVENT_CLASS_TYPE(AppTick)
			EVENT_CLASS_CATEGORY(Application)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		EVENT_CLASS_TYPE(AppUpdate)
			EVENT_CLASS_CATEGORY(Application)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		EVENT_CLASS_TYPE(AppRender)
			EVENT_CLASS_CATEGORY(Application)
	};
}

#endif
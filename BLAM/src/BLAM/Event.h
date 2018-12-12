#pragma once

#include "Core.h"
#include "Event/Types.h"

#include <string>
#include <functional>

#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
#include <sstream>
#endif // defined BLAM_DEBUG or BLAM_RELEASE

namespace Blam {

	class BLAM_API Event
	{
		friend class Handler;
	public:
		virtual EventType GetEventType() const = 0;

		inline bool IsOfType(EventType type)
		{
			return (int) (GetEventType() & type);
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const = 0;
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	protected:
		bool active = true;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) { return os << e.ToString(); }

	/* --- Window Events --- */

	class BLAM_API WindowResizedEvent
		: public Event
	{
	public:
		WindowResizedEvent(unsigned int width, unsigned int height)
			: width(width), height(height) {}

		virtual EventType GetEventType() const override
		{
			return EventType::Window | EventType::WindowResized;
		}

		inline unsigned int GetWidth() const { return width; }
		inline unsigned int GetHeight() const { return height; }

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "WindowResizedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << width << ", " << height;
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE

	private:
		unsigned int width, height;
	};

	class BLAM_API WindowMovedEvent
		: public Event
	{
	public:
		WindowMovedEvent() {}

		virtual EventType GetEventType() const override
		{
			return EventType::Window | EventType::WindowMoved;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "WindowMovedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName();
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API WindowFocusedEvent
		: public Event
	{
	public:
		WindowFocusedEvent() {}

		virtual EventType GetEventType() const override
		{
			return EventType::Window | EventType::WindowFocused;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "WindowFocusedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName();
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API WindowLostFocusEvent
		: public Event
	{
	public:
		WindowLostFocusEvent() {}

		virtual EventType GetEventType() const override
		{
			return EventType::Window | EventType::WindowLostFocus;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "WindowLostFocusEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName();
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API WindowClosedEvent
		: public Event
	{
	public:
		WindowClosedEvent() {}

		virtual EventType GetEventType() const override
		{
			return EventType::Window | EventType::WindowClosed;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "WindowClosedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName();
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};


	/* --- Application Events --- */

	class BLAM_API AppTickedEvent
		: public Event
	{
	public:
		AppTickedEvent() {}

		virtual EventType GetEventType() const override
		{
			return EventType::Application | EventType::AppTicked;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "AppTickedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName();
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API AppUpdatedEvent
		: public Event
	{
	public:
		AppUpdatedEvent() {}

		virtual EventType GetEventType() const override
		{
			return EventType::Application | EventType::AppUpdated;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "AppUpdatedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName();
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API AppRenderedEvent
		: public Event
	{
	public:
		AppRenderedEvent() {}

		virtual EventType GetEventType() const override
		{
			return EventType::Application | EventType::AppRendered;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "AppRenderedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName();
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};


	/* --- Keyboard Events --- */

	class BLAM_API KeyPressedEvent
		: public Event
	{
	public:
		KeyPressedEvent(int keyCode, unsigned int repeat)
			: keyCode(keyCode), repeat(repeat) {}

		int keyCode;
		unsigned int repeat;

		virtual EventType GetEventType() const override
		{
			return EventType::Keyboard | EventType::KeyPressed;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "KeyPressedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << keyCode << " (" << repeat << ")";
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API KeyReleasedEvent
		: public Event
	{
	public:
		KeyReleasedEvent(int keyCode)
			: keyCode(keyCode) {}

		int keyCode;

		virtual EventType GetEventType() const override
		{
			return EventType::Keyboard | EventType::KeyReleased;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "KeyReleasedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << keyCode;
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};


	/* --- Mouse Events --- */

	class BLAM_API MouseButtonPressedEvent
		: public Event
	{
	public:
		MouseButtonPressedEvent(int button)
			: button(button) {}

		int button;

		virtual EventType GetEventType() const override
		{
			return EventType::Mouse | EventType::MouseButtonPressed;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "MouseButtonPressedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << button;
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API MouseButtonReleasedEvent
		: public Event
	{
	public:
		MouseButtonReleasedEvent(int button)
			: button(button) {}

		int button;

		virtual EventType GetEventType() const override
		{
			return EventType::Mouse | EventType::MouseButtonReleased;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "MouseButtonReleasedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << button;
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API MouseMovedEvent
		: public Event
	{
	public:
		MouseMovedEvent(int x, int y)
			: x(x), y(y) {}

		int x, y;

		virtual EventType GetEventType() const override
		{
			return EventType::Mouse | EventType::MouseMoved;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "MouseMovedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << x << ", " << y;
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};

	class BLAM_API MouseScrolledEvent
		: public Event
	{
	public:
		MouseScrolledEvent(int down, int right)
			: down(down), right(right) {}

		int down, right;

		virtual EventType GetEventType() const override
		{
			return EventType::Mouse | EventType::MouseScrolled;
		}

		#if defined(BLAM_DEBUG) | defined(BLAM_RELEASE)
		virtual const char* GetName() const override
		{
			return "MouseScrolledEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << down << ", " << right;
			return ss.str();
		}
		#endif // defined BLAM_DEBUG or BLAM_RELEASE
	};
	
}
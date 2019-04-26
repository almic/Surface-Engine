#pragma once

#include "Core.h"
#include "Event/Types.h"

namespace Surface {

#define SURF_EVENT_TYPE(t) static EventType GetEventType() { return t; }\
						   virtual bool IsOfCategory(EventType type) override { return (bool) (GetEventType() & type & EventType::CategoryMask); }\
						   virtual bool IsOfType(EventType type) override { return GetEventType() == type; }

	class SURF_API Event
	{
		friend class Handler;
	public:
		static EventType GetEventType() { return EventType::None; }
		virtual bool IsOfCategory(EventType type) { return (bool) (GetEventType() & type & EventType::CategoryMask); }
		virtual bool IsOfType(EventType type) { return GetEventType() == type; }

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const = 0;
		#endif // defined SURF_DEBUG or SURF_RELEASE
	protected:
		bool active = true;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) { return os << e.ToString(); }

	/* --- Window Events --- */

	class SURF_API WindowEvent
		: public Event
	{
	public:
		WindowEvent() {}

		SURF_EVENT_TYPE(EventType::Window)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
		virtual const char* GetName() const override
		{
			return "WindowEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName();
			return ss.str();
		}
		#endif // defined SURF_DEBUG or SURF_RELEASE

	};

	class SURF_API WindowResizedEvent
		: public Event
	{
	public:
		WindowResizedEvent(unsigned int width, unsigned int height)
			: width(width), height(height) {}

		SURF_EVENT_TYPE(EventType::WindowResized)

		inline unsigned int GetWidth() const { return width; }
		inline unsigned int GetHeight() const { return height; }

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE

	private:
		unsigned int width, height;
	};

	class SURF_API WindowMovedEvent
		: public Event
	{
	public:
		WindowMovedEvent(int xPos, int yPos)
			: xPos(xPos), yPos(yPos) {}

		SURF_EVENT_TYPE(EventType::WindowMoved)

		inline int GetX() const { return xPos; }
		inline int GetY() const { return yPos; }

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
		virtual const char* GetName() const override
		{
			return "WindowMovedEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << xPos << ", " << yPos;
			return ss.str();
		}
		#endif // defined SURF_DEBUG or SURF_RELEASE
	private:
		int xPos, yPos;
	};

	class SURF_API WindowFocusedEvent
		: public Event
	{
	public:
		WindowFocusedEvent() {}

		SURF_EVENT_TYPE(EventType::WindowFocused)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

	class SURF_API WindowLostFocusEvent
		: public Event
	{
	public:
		WindowLostFocusEvent() {}

		SURF_EVENT_TYPE(EventType::WindowLostFocus)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

	class SURF_API WindowClosedEvent
		: public Event
	{
	public:
		WindowClosedEvent() {}

		SURF_EVENT_TYPE(EventType::WindowClosed)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};


	/* --- Application Events --- */

	class SURF_API AppTickedEvent
		: public Event
	{
	public:
		AppTickedEvent() {}

		SURF_EVENT_TYPE(EventType::AppTicked)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

	class SURF_API AppUpdatedEvent
		: public Event
	{
	public:
		AppUpdatedEvent() {}

		SURF_EVENT_TYPE(EventType::AppUpdated)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

	class SURF_API AppRenderedEvent
		: public Event
	{
	public:
		AppRenderedEvent() {}

		SURF_EVENT_TYPE(EventType::AppRendered)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};


	/* --- Keyboard Events --- */

	class SURF_API KeyPressedEvent
		: public Event
	{
	public:
		KeyPressedEvent(int keyCode, unsigned int repeat)
			: keyCode(keyCode), repeat(repeat) {}

		int keyCode;
		unsigned int repeat;

		SURF_EVENT_TYPE(EventType::KeyPressed)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

	class SURF_API KeyReleasedEvent
		: public Event
	{
	public:
		KeyReleasedEvent(int keyCode)
			: keyCode(keyCode) {}

		int keyCode;

		SURF_EVENT_TYPE(EventType::KeyReleased)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};


	/* --- Mouse Events --- */

	class SURF_API MouseButtonPressedEvent
		: public Event
	{
	public:
		MouseButtonPressedEvent(int button)
			: button(button) {}

		int button;

		SURF_EVENT_TYPE(EventType::MouseButtonPressed)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

	class SURF_API MouseButtonReleasedEvent
		: public Event
	{
	public:
		MouseButtonReleasedEvent(int button)
			: button(button) {}

		int button;

		SURF_EVENT_TYPE(EventType::MouseButtonReleased)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

	class SURF_API MouseMovedEvent
		: public Event
	{
	public:
		MouseMovedEvent(int x, int y)
			: x(x), y(y) {}

		int x, y;

		SURF_EVENT_TYPE(EventType::MouseMoved)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
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
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

	class SURF_API MouseScrolledEvent
		: public Event
	{
	public:
		MouseScrolledEvent(int up, int right)
			: up(up), right(right) {}

		int up, right;

		SURF_EVENT_TYPE(EventType::MouseScrolled)

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
		virtual const char* GetName() const override
		{
			return "MouseScrolledEvent";
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << GetName() << ": " << up << ", " << right;
			return ss.str();
		}
		#endif // defined SURF_DEBUG or SURF_RELEASE
	};

}

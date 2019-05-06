#pragma once

#include <string>

namespace Surface {

	enum class EventType : unsigned
	{
		/*
		Each event type can hold 15 unique sub-types.
		This system is used to allow handlers to quickly check what type an
		event actually is without having to use long if-else chains.
		This is just for code readability when making handlers.
		*/

		// None-type event. Useful for... nothing?
		None = 0,

		// Mask for category, for the IsOfType() method in Events
		CategoryMask = 0xF0,

		// Window events. Anything that happens to the physical window "box"
		Window = 0x10,
		WindowResized,
		WindowMoved,
		WindowFocused,
		WindowLostFocus,
		WindowClosed,

		// Application events.
		Application = 0x20,
		AppTicked,
		AppUpdated,
		AppRendered,

		// Keyboard events. Keys are being touched.
		Keyboard = 0x40,
		KeyPressed,
		KeyReleased,
		Character,

		// Mouse events. Mouse is being touched.
		Mouse = 0x80,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseScrolled,

		// Input mask
		Input = Keyboard | Mouse
	};

	inline EventType operator&(EventType a, EventType b)
	{
		return static_cast<EventType> (
			static_cast<std::underlying_type<EventType>::type>(a) &
			static_cast<std::underlying_type<EventType>::type>(b)
			);
	}

	inline EventType operator|(EventType a, EventType b)
	{
		return static_cast<EventType> (
			static_cast<std::underlying_type<EventType>::type>(a) |
			static_cast<std::underlying_type<EventType>::type>(b)
			);
	}

}

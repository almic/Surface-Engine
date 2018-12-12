#pragma once

#include "BLAM/Event.h"

namespace Blam {

	class Handler
	{
		template<typename T>
		using EventFunc = std::function<bool(T&)>;

	public:
		Handler(Event& e) : event(e) {}

		template<typename T>
		bool Fire(EventFunc<T> func)
		{
			if (event.GetEventType() == T::GetStaticType())
			{
				event.active = func(*(T*)&event);
				return true;
			}
			return false;
		}

	private:
		Event& event;
	};

}
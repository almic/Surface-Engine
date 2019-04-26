#pragma once

#include "Surface/Event.h"

namespace Surface {

	class SURF_API Handler
	{
		template<typename T>
		using EventFunc = std::function<bool(T&)>;

	public:
		Handler(Event& e) : event(e) {}

		template<typename T>
		bool Fire(EventFunc<T> func)
		{
			EventType type = T::GetEventType();
			if (event.active && event.IsOfType(type) || type == (type & EventType::CategoryMask) && event.IsOfCategory(type))
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

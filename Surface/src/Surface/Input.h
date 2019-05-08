#pragma once

#include "Core.h"

namespace Surface {

	typedef int Key;
	
	class SURF_API Input
	{
	public:
		static bool IsKeyPressed(int key);
		static bool IsCursorButtonPressed(int button);
		static std::pair<float, float> GetCursorXY();
	};

	// We effectively copy the GLFW key bindings, but with some extra names
	enum : Key
	{
		KEY_UNKNOWN       = -1,
		KEY_SPACE         = 32,
		KEY_APOSTROPHE    = 39,
		KEY_COMMA         = 44,
		KEY_LT            = KEY_COMMA,
		KEY_LESS_THAN     = KEY_COMMA,
		KEY_MINUS         = 45,
		KEY_UNDERSCORE    = KEY_MINUS,
		KEY_PERIOD        = 46,
		KEY_GT            = KEY_PERIOD,
		KEY_GREATER_THAN  = KEY_PERIOD,
		KEY_SLASH         = 47,
		KEY_FORWARDSLASH  = KEY_SLASH,
		KEY_QUESTION      = KEY_SLASH,
		KEY_0             = 48,
		KEY_1             = 49,
		KEY_2             = 50,
		KEY_3             = 51,
		KEY_4             = 52,
		KEY_5             = 53,
		KEY_6             = 54,
		KEY_7             = 55,
		KEY_8             = 56,
		KEY_9             = 57,
		KEY_SEMICOLON     = 59,
		KEY_COLON         = KEY_SEMICOLON,
		KEY_EQUAL         = 61,
		KEY_PLUS          = KEY_EQUAL,
		KEY_A             = 65,
		KEY_B             = 66,
		KEY_C             = 67,
		KEY_D             = 68,
		KEY_E             = 69,
		KEY_F             = 70,
		KEY_G             = 71,
		KEY_H             = 72,
		KEY_I             = 73,
		KEY_J             = 74,
		KEY_K             = 75,
		KEY_L             = 76,
		KEY_M             = 77,
		KEY_N             = 78,
		KEY_O             = 79,
		KEY_P             = 80,
		KEY_Q             = 81,
		KEY_R             = 82,
		KEY_S             = 83,
		KEY_T             = 84,
		KEY_U             = 85,
		KEY_V             = 86,
		KEY_W             = 87,
		KEY_X             = 88,
		KEY_Y             = 89,
		KEY_Z             = 90,
		KEY_LEFT_BRACKET  = 91,
		KEY_BACKSLASH     = 92,
		KEY_RIGHT_BRACKET = 93,
		KEY_GRAVE         = 96,
		KEY_GRAVE_ACCENT  = KEY_GRAVE,
		KEY_BACKTICK      = KEY_GRAVE,
		KEY_BACKQUOTE     = KEY_GRAVE,
		KEY_WORLD_1       = 161,
		KEY_WORLD_2       = 162,
		KEY_ESCAPE        = 256,
		KEY_ENTER         = 257,
		KEY_TAB           = 258,
		KEY_BACKSPACE     = 259,
		KEY_INSERT        = 260,
		KEY_DELETE        = 261,
		KEY_RIGHT         = 262,
		KEY_LEFT          = 263,
		KEY_DOWN          = 264,
		KEY_UP            = 265,
		KEY_PAGE_UP       = 266,
		KEY_PAGE_DOWN     = 267,
		KEY_HOME          = 268,
		KEY_END           = 269,
		KEY_CAPS_LOCK     = 280,
		KEY_SCROLL_LOCK   = 281,
		KEY_NUM_LOCK      = 282,
		KEY_KP_CLEAR      = KEY_NUM_LOCK,
		KEY_PRINT_SCREEN  = 283,
		KEY_PAUSE         = 284,
		KEY_F1            = 290,
		KEY_F2            = 291,
		KEY_F3            = 292,
		KEY_F4            = 293,
		KEY_F5            = 294,
		KEY_F6            = 295,
		KEY_F7            = 296,
		KEY_F8            = 297,
		KEY_F9            = 298,
		KEY_F10           = 299,
		KEY_F11           = 300,
		KEY_F12           = 301,
		KEY_F13           = 302,
		KEY_F14           = 303,
		KEY_F15           = 304,
		KEY_F16           = 305,
		KEY_F17           = 306,
		KEY_F18           = 307,
		KEY_F19           = 308,
		KEY_F20           = 309,
		KEY_F21           = 310,
		KEY_F22           = 311,
		KEY_F23           = 312,
		KEY_F24           = 313,
		KEY_F25           = 314,
		KEY_KP_0          = 320,
		KEY_KP_INSERT     = KEY_KP_0,
		KEY_KP_1          = 321,
		KEY_KP_END        = KEY_KP_1,
		KEY_KP_2          = 322,
		KEY_KP_DOWN       = KEY_KP_2,
		KEY_KP_3          = 323,
		KEY_KP_PAGE_DOWN  = KEY_KP_3,
		KEY_KP_4          = 324,
		KEY_KP_LEFT       = KEY_KP_4,
		KEY_KP_5          = 325,
		KEY_KP_6          = 326,
		KEY_KP_RIGHT      = KEY_KP_6,
		KEY_KP_7          = 327,
		KEY_KP_HOME       = KEY_KP_7,
		KEY_KP_8          = 328,
		KEY_KP_UP         = KEY_KP_8,
		KEY_KP_9          = 329,
		KEY_KP_PAGE_UP    = KEY_KP_9,
		KEY_KP_DECIMAL    = 330,
		KEY_KP_DELETE     = KEY_KP_DECIMAL,
		KEY_KP_DIVIDE     = 331,
		KEY_KP_MULTIPLY   = 332,
		KEY_KP_SUBTRACT   = 333,
		KEY_KP_ADD        = 334,
		KEY_KP_ENTER      = 335,
		KEY_KP_EQUAL      = 336,
		KEY_LEFT_SHIFT    = 340,
		KEY_LEFT_CONTROL  = 341,
		KEY_LEFT_ALT      = 342,
		KEY_LEFT_OPTION   = KEY_LEFT_ALT,
		KEY_LEFT_SUPER    = 343,
		KEY_WINDOWS       = KEY_LEFT_SUPER,
		KEY_LEFT_COMMAND  = KEY_LEFT_SUPER,
		KEY_RIGHT_SHIFT   = 344,
		KEY_RIGHT_CONTROL = 345,
		KEY_RIGHT_ALT     = 346,
		KEY_RIGHT_OPTION  = KEY_RIGHT_ALT,
		KEY_RIGHT_SUPER   = 347,
		KEY_FUNCTION      = KEY_RIGHT_SUPER,
		KEY_RIGHT_COMMAND = KEY_RIGHT_SUPER,
		KEY_MENU          = 348
	};

	class SURF_API KeyMap
	{
	public:
		typedef std::vector<std::pair<int, int>> Map;
		static const int AllKeys[121];

		KeyMap()
		{
			map = new Map();
			for (Key key : AllKeys)
			{
				map->push_back({ key, key });
			}
		}

		KeyMap(const Map& map)
		{
			this->map = new Map();
			for (auto pair : map)
			{
				this->map->push_back(pair);
			}
		}

		~KeyMap()
		{
			delete map;
		}

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
			#define WARN_FROM(key) SURF_CORE_WARN("The \"from\" key \"{0}\" is invalid! Bindings for this key will not work.", KeyToString(key))
			#define WARN_TO(key)   SURF_CORE_WARN("The \"to\" key \"{0}\" is invalid! Bindings of this key will not work.", KeyToString(key))
			#define WARN_FIND(key) SURF_CORE_WARN("Could not find \"{0}\" in KeyMap! -1 Returned.", KeyToString(key))
		#else
			#define WARN_FROM(key)
			#define WARN_TO(key)
			#define WARN_FIND(key)
		#endif

		// This "function" makes sure the passed keys are valid keys, if not then it will return the "result" and may log
		#define CHECK_KEYS(from, to, result)        \
		{                                           \
			bool found_from = false,                \
				 found_to = false;                  \
			for (Key key : AllKeys)                 \
			{                                       \
				if (key == from)                    \
					found_from = true;              \
				if (key == to)                      \
					found_to = true;                \
				if (found_from && found_to) break;  \
			}                                       \
			if (!found_from || !found_to)           \
			{                                       \
				if (!found_from)                    \
					WARN_FROM(from);                \
				if (!found_to)                      \
					WARN_TO(to);                    \
				return result;                      \
			}                                       \
		}

		inline KeyMap Add(Key from, Key to, bool shouldOverride = true) { return Add({ from, to }, shouldOverride); }

		inline KeyMap Add(std::pair<Key, Key> pair, bool shouldOverride = true) {
			auto[from, to] = pair;
			CHECK_KEYS(from, to, *this);

			// Make sure that "from" isn't already defined, and overrides it if needed
			Map::iterator index = map->begin();
			Map::iterator last = map->end();
			for (; index != last; index++)
			{
				auto[from_b, _] = *index;
				if (from_b == from)
				{
					// Override the key
					if (shouldOverride)
						map->erase(index);
					else
						return *this;
					break;
				}
			}
			map->push_back({ from, to });
			return *this;
		}

		// This returns the "from" key, use the [] syntax to get the "to" key
		inline Key Find(Key to)
		{
			std::vector<Key> keys = FindAll(to, 1);
			if (keys.size() > 0) return keys[0];
			else return KEY_UNKNOWN;
		}

		// This returns a vector of all "from" keys that map to the given key, with optional max
		inline std::vector<Key> FindAll(Key to, unsigned int max = 0)
		{
			std::vector<Key> result = {};
			CHECK_KEYS(KEY_SPACE, to, result);

			unsigned int size = 0;
			for (auto[from, to_b] : *map)
			{
				// if (to_b || !to_b), sorry I had to
				if (to_b == to)
				{
					result.push_back(from);
					if (++size == max) break;
				}
			}
			return result;
		}

		// Use this to read and change bindings
		inline Key operator[](Key from)
		{
			UNKNOWN = -1;
			if (from > 0)
				for (auto& [from_b, to] : *map)
				{
					if (from_b == from) return to;
				}
			WARN_FIND(from);
			return UNKNOWN;
		}

		#if defined(SURF_DEBUG) | defined(SURF_RELEASE)
		inline static std::string KeyToString(Key key)
		{
			switch (key)
			{
			case KEY_SPACE:         return "KEY_SPACE";
			case KEY_APOSTROPHE:    return "KEY_APOSTROPHE";
			case KEY_COMMA:         return "KEY_COMMA";
			case KEY_MINUS:         return "KEY_MINUS";
			case KEY_PERIOD:        return "KEY_PERIOD";
			case KEY_SLASH:         return "KEY_SLASH";
			case KEY_0:             return "KEY_0";
			case KEY_1:             return "KEY_1";
			case KEY_2:             return "KEY_2";
			case KEY_3:             return "KEY_3";
			case KEY_4:             return "KEY_4";
			case KEY_5:             return "KEY_5";
			case KEY_6:             return "KEY_6";
			case KEY_7:             return "KEY_7";
			case KEY_8:             return "KEY_8";
			case KEY_9:             return "KEY_9";
			case KEY_SEMICOLON:     return "KEY_SEMICOLON";
			case KEY_EQUAL:         return "KEY_EQUAL";
			case KEY_A:             return "KEY_A";
			case KEY_B:             return "KEY_B";
			case KEY_C:             return "KEY_C";
			case KEY_D:             return "KEY_D";
			case KEY_E:             return "KEY_E";
			case KEY_F:             return "KEY_F";
			case KEY_G:             return "KEY_G";
			case KEY_H:             return "KEY_H";
			case KEY_I:             return "KEY_I";
			case KEY_J:             return "KEY_J";
			case KEY_K:             return "KEY_K";
			case KEY_L:             return "KEY_L";
			case KEY_M:             return "KEY_M";
			case KEY_N:             return "KEY_N";
			case KEY_O:             return "KEY_O";
			case KEY_P:             return "KEY_P";
			case KEY_Q:             return "KEY_Q";
			case KEY_R:             return "KEY_R";
			case KEY_S:             return "KEY_S";
			case KEY_T:             return "KEY_T";
			case KEY_U:             return "KEY_U";
			case KEY_V:             return "KEY_V";
			case KEY_W:             return "KEY_W";
			case KEY_X:             return "KEY_X";
			case KEY_Y:             return "KEY_Y";
			case KEY_Z:             return "KEY_Z";
			case KEY_LEFT_BRACKET:  return "KEY_LEFT_BRACKET";
			case KEY_BACKSLASH:     return "KEY_BACKSLASH";
			case KEY_RIGHT_BRACKET: return "KEY_RIGHT_BRACKET";
			case KEY_GRAVE:         return "KEY_GRAVE";
			case KEY_WORLD_1:       return "KEY_WORLD_1";
			case KEY_WORLD_2:       return "KEY_WORLD_2";
			case KEY_ESCAPE:        return "KEY_ESCAPE";
			case KEY_ENTER:         return "KEY_ENTER";
			case KEY_TAB:           return "KEY_TAB";
			case KEY_BACKSPACE:     return "KEY_BACKSPACE";
			case KEY_INSERT:        return "KEY_INSERT";
			case KEY_DELETE:        return "KEY_DELETE";
			case KEY_RIGHT:         return "KEY_RIGHT";
			case KEY_LEFT:          return "KEY_LEFT";
			case KEY_DOWN:          return "KEY_DOWN";
			case KEY_UP:            return "KEY_UP";
			case KEY_PAGE_UP:       return "KEY_PAGE_UP";
			case KEY_PAGE_DOWN:     return "KEY_PAGE_DOWN";
			case KEY_HOME:          return "KEY_HOME";
			case KEY_END:           return "KEY_END";
			case KEY_CAPS_LOCK:     return "KEY_CAPS_LOCK";
			case KEY_SCROLL_LOCK:   return "KEY_SCROLL_LOCK";
			case KEY_NUM_LOCK:      return "KEY_NUM_LOCK";
			case KEY_PRINT_SCREEN:  return "KEY_PRINT_SCREEN";
			case KEY_PAUSE:         return "KEY_PAUSE";
			case KEY_F1:            return "KEY_F1";
			case KEY_F2:            return "KEY_F2";
			case KEY_F3:            return "KEY_F3";
			case KEY_F4:            return "KEY_F4";
			case KEY_F5:            return "KEY_F5";
			case KEY_F6:            return "KEY_F6";
			case KEY_F7:            return "KEY_F7";
			case KEY_F8:            return "KEY_F8";
			case KEY_F9:            return "KEY_F9";
			case KEY_F10:           return "KEY_F10";
			case KEY_F11:           return "KEY_F11";
			case KEY_F12:           return "KEY_F12";
			case KEY_F13:           return "KEY_F13";
			case KEY_F14:           return "KEY_F14";
			case KEY_F15:           return "KEY_F15";
			case KEY_F16:           return "KEY_F16";
			case KEY_F17:           return "KEY_F17";
			case KEY_F18:           return "KEY_F18";
			case KEY_F19:           return "KEY_F19";
			case KEY_F20:           return "KEY_F20";
			case KEY_F21:           return "KEY_F21";
			case KEY_F22:           return "KEY_F22";
			case KEY_F23:           return "KEY_F23";
			case KEY_F24:           return "KEY_F24";
			case KEY_F25:           return "KEY_F25";
			case KEY_KP_0:          return "KEY_KP_0";
			case KEY_KP_1:          return "KEY_KP_1";
			case KEY_KP_2:          return "KEY_KP_2";
			case KEY_KP_3:          return "KEY_KP_3";
			case KEY_KP_4:          return "KEY_KP_4";
			case KEY_KP_5:          return "KEY_KP_5";
			case KEY_KP_6:          return "KEY_KP_6";
			case KEY_KP_7:          return "KEY_KP_7";
			case KEY_KP_8:          return "KEY_KP_8";
			case KEY_KP_9:          return "KEY_KP_9";
			case KEY_KP_DECIMAL:    return "KEY_KP_DECIMAL";
			case KEY_KP_DIVIDE:     return "KEY_KP_DIVIDE";
			case KEY_KP_MULTIPLY:   return "KEY_KP_MULTIPLY";
			case KEY_KP_SUBTRACT:   return "KEY_KP_SUBTRACT";
			case KEY_KP_ADD:        return "KEY_KP_ADD";
			case KEY_KP_ENTER:      return "KEY_KP_ENTER";
			case KEY_KP_EQUAL:      return "KEY_KP_EQUAL";
			case KEY_LEFT_SHIFT:    return "KEY_LEFT_SHIFT";
			case KEY_LEFT_CONTROL:  return "KEY_LEFT_CONTROL";
			case KEY_LEFT_ALT:      return "KEY_LEFT_ALT";
			case KEY_LEFT_SUPER:    return "KEY_LEFT_SUPER";
			case KEY_RIGHT_SHIFT:   return "KEY_RIGHT_SHIFT";
			case KEY_RIGHT_CONTROL: return "KEY_RIGHT_CONTROL";
			case KEY_RIGHT_ALT:     return "KEY_RIGHT_ALT";
			case KEY_RIGHT_SUPER:   return "KEY_RIGHT_SUPER";
			case KEY_MENU:          return "KEY_MENU";
			case KEY_UNKNOWN:
			default:                return "KEY_UNKNOWN";
			}
		}
		#endif

	private:
		static Key UNKNOWN;
		Map* map;
	};

#undef CHECK_KEYS
#undef WARN_FIND
#undef WARN_TO
#undef WARN_FROM

}

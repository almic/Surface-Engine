#pragma once

#include "Core.h"
#include "Log.h"

#include <fstream>

namespace Surface {

	SURF_API std::string ReadFile(const char* file);

	SURF_API bool WriteFile(const char* file, const char* data);

}

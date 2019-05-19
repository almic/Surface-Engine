#include "spch.h"

#include "IO.h"

namespace Surface {

	std::string ReadFile(const char* file)
	{
		std::ifstream f(file);
		if (!f.is_open())
		{
			SURF_CORE_WARN("Failed to open file \"{0}\" for reading", file);
			return "";
		}

		std::string result, line;
		while (std::getline(f, line)) result += line;

		if (!f.good())
		{
			SURF_CORE_WARN("There was a problem while reading the file \"{0}\", incomplete result may have been returned: exception {1}", file, f.exceptions());
		}

		f.close();
		return result;
	}

	bool WriteFile(const char* file, const char* data)
	{
		std::ofstream f(file);
		if (!f.is_open())
		{
			SURF_CORE_WARN("Failed to open file \"{0}\" for writing", file);
			return false;
		}

		f << data;

		if (!f.good())
		{
			SURF_CORE_WARN("There was a problem while writing to the file \"{0}\": exception {1}", file, f.exceptions());
			f.close();
			return false;
		}

		f.close();
		return true;
	}

}

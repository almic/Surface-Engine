#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Surface {

	class SURF_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

#define SURF_CORE_TRACE(...) ::Surface::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SURF_CORE_INFO(...)  ::Surface::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SURF_CORE_WARN(...)  ::Surface::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SURF_CORE_ERROR(...) ::Surface::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SURF_CORE_FATAL(...) ::Surface::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define SURF_TRACE(...) ::Surface::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SURF_INFO(...)  ::Surface::Log::GetClientLogger()->info(__VA_ARGS__)
#define SURF_WARN(...)  ::Surface::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SURF_ERROR(...) ::Surface::Log::GetClientLogger()->error(__VA_ARGS__)
#define SURF_FATAL(...) ::Surface::Log::GetClientLogger()->fatal(__VA_ARGS__)

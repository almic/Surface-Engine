#pragma once

#include <memory>
#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Blam {

	class BLAM_API Log
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

#define BLAM_CORE_TRACE(...) ::Blam::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define BLAM_CORE_INFO(...)  ::Blam::Log::GetCoreLogger()->info(__VA_ARGS__)
#define BLAM_CORE_WARN(...)  ::Blam::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define BLAM_CORE_ERROR(...) ::Blam::Log::GetCoreLogger()->error(__VA_ARGS__)
#define BLAM_CORE_FATAL(...) ::Blam::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define BLAM_TRACE(...) ::Blam::Log::GetClientLogger()->trace(__VA_ARGS__)
#define BLAM_INFO(...)  ::Blam::Log::GetClientLogger()->info(__VA_ARGS__)
#define BLAM_WARN(...)  ::Blam::Log::GetClientLogger()->warn(__VA_ARGS__)
#define BLAM_ERROR(...) ::Blam::Log::GetClientLogger()->error(__VA_ARGS__)
#define BLAM_FATAL(...) ::Blam::Log::GetClientLogger()->fatal(__VA_ARGS__)
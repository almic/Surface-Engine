#pragma once

#include <filesystem>

namespace Surface::FS
{

// Returns a path to the system user's application data directory
std::filesystem::path user_app_data_path();

// Returns a path to the system's global application data directory
std::filesystem::path sys_app_data_path();

} // namespace Surface::FS

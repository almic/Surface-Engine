#include "file.h"

using std::filesystem::path;

// Implement per platform
namespace Surface::FS
{

std::unique_ptr<char[]> user_app_data_path_str = nullptr;
std::unique_ptr<char[]> sys_app_data_path_str = nullptr;

char* platform_user_app_data_path();

char* platform_sys_app_data_path();

inline static const char* get_user_app_data_path()
{
    if (user_app_data_path_str == nullptr)
    {
        user_app_data_path_str = std::unique_ptr<char[]>(platform_user_app_data_path());
    }

    return user_app_data_path_str.get();
}

inline static const char* get_sys_app_data_path()
{
    if (sys_app_data_path_str == nullptr)
    {
        sys_app_data_path_str = std::unique_ptr<char[]>(platform_sys_app_data_path());
    }

    return sys_app_data_path_str.get();
}

} // namespace Surface::FS

////////////////////////////////////////////////
//              Windows Platform              //
////////////////////////////////////////////////
#ifdef PLATFORM_WINDOWS

// TODO: include the exact windows headers actually used by Surface Window
//       I've gone ahead and manually included the transitive headers anyway
//       But Windows.h surely includes far more stuff than we use here
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <shlobj.h>

namespace Surface::FS
{

static char* windows_path(const KNOWNFOLDERID& path)
{
    PWSTR pwstr;

    auto result = SHGetKnownFolderPath(path, KF_FLAG_DEFAULT, nullptr, &pwstr);
    if (result != S_OK)
    {
        CoTaskMemFree(pwstr);
        return nullptr;
    }

    // Convert wchar_t to char
    int length = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, pwstr, -1, nullptr, 0, 0, 0);
    char* cstr = new char[length];
    WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, pwstr, -1, cstr, length, 0, 0);
    CoTaskMemFree(pwstr);

    // may not be needed
    cstr[length - 1] = '\0';

    return cstr;
}

char* platform_user_app_data_path()
{
    return windows_path(FOLDERID_RoamingAppData);
}

char* platform_sys_app_data_path()
{
    return windows_path(FOLDERID_ProgramFiles);
}

} // namespace Surface::FS
#endif

namespace Surface::FS
{

path user_app_data_path()
{
    return path(get_user_app_data_path());
}

path sys_app_data_path()
{
    return path(get_sys_app_data_path());
}

} // namespace Surface::FS

project "Surface"
    -- Basic options
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    -- build outputs
    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin/int/" .. outputdir .. "/%{prj.name}")

    -- project files
    includedirs
    {
        "src",
    }

    files
    {
        "src/**.h",
        "src/**.cpp",
    }

    -- system options

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "PLATFORM_WINDOWS"
        }


    -- build configurations

    filter "configurations:Debug"
        runtime "Debug"

        symbols "on"

        defines
        {
            "DEBUG"
        }

    filter "configurations:Release"
        runtime "Release"

        optimize "on"
        symbols "on"

        defines
        {
            "RELEASE"
        }

    filter "configurations:Dist"
        runtime "Release"

        optimize "on"

        defines
        {
            "DIST"
        }

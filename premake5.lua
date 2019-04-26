workspace "Surface"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Surface/vendor/GLFW/include"

include "Surface/vendor/GLFW"

project "Surface"
	location "Surface"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "spch.h"
	pchsource "Surface/src/spch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}"
	}

	links
	{
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "off"
		systemversion "latest"

		defines
		{
			"SURF_PLATFORM_WINDOWS",
			"SURF_BUILD"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "SURF_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "SURF_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "SURF_DIST"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Surface/vendor/spdlog/include",
		"Surface/src"
	}

	links
	{
		"Surface"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "off"
		systemversion "latest"

		defines
		{
			"SURF_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "SURF_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "SURF_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "SURF_DIST"
		optimize "On"

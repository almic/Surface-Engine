workspace "Surface"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }
	startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Surface/vendor/GLFW/include"
IncludeDir["Glad"] = "Surface/vendor/Glad/include"
IncludeDir["imgui"] = "Surface/vendor/imgui"
IncludeDir["spdlog"] = "Surface/vendor/spdlog/include"

group "Dependencies"
	include "Surface/vendor/GLFW"
	include "Surface/vendor/Glad"
	include "Surface/vendor/imgui"

group ""

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
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.spdlog}"
	}

	links
	{
		"GLFW",
		"Glad",
		"imgui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "off"
		systemversion "latest"

		defines
		{
			"SURF_PLATFORM_WINDOWS",
			"SURF_BUILD",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
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
		"Surface/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.spdlog}"
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

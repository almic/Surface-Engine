workspace "Blam"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Blam"
	location "BLAM"
	kind "SharedLib"
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
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"BLAM_PLATFORM_WINDOWS",
			"BLAM_BUILD"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "BLAM_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "BLAM_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "BLAM_DIST"
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
		"BLAM/vendor/spdlog/include",
		"BLAM/src"
	}

	links
	{
		"BLAM"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"BLAM_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "BLAM_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "BLAM_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "BLAM_DIST"
		optimize "On"
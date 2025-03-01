workspace "Surface"
    -- global settings
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "Sandbox"

    -- windows build settings
    filter "system:windows"
        buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

-- global output directory
outputdir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

-- test project
group "Sandbox"
    include "Sandbox/build.lua"
group ""

-- main project
include "Surface/build.lua"

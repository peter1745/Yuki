require "Premake/premake-vscode/vscode"

workspace "Yuki"
	configurations { "Debug", "RelWithDebug", "Release" }
	architecture "x86_64"

	language "C++"
	cppdialect "C++20"

	targetdir "Build/%{cfg.buildcfg}"
	objdir "Intermediates/%{cfg.buildcfg}"

	externalanglebrackets "On"
	externalwarnings "Off"
	warnings "Off"

	flags { "MultiProcessorCompile" }

	filter "configurations:Debug"
		symbols "On"
		optimize "Off"

	filter "configurations:RelWithDebug"
		symbols "On"
		optimize "Debug"

	filter "configurations:Release"
		symbols "Off"
		optimize "Full"

	filter "system:windows"
		defines { "_CRT_SECURE_NO_WARNINGS" }

		disablewarnings {
            "4100" -- Unreferenced Formal Parameter
        }

group "ThirdParty"
include "ThirdParty/"
group ""

include "Yuki/"
include "Sandbox/"

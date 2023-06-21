require "Premake/premake-vscode/vscode"

workspace "Yuki"
	configurations { "RelWithDebug", "Debug", "Release" }
	architecture "x86_64"

	language "C++"
	cppdialect "C++20"

	targetdir "Out/Bin/%{cfg.buildcfg}"
	objdir "Out/Intermediates/%{cfg.buildcfg}"

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
            "4100", -- Unreferenced Formal Parameter
			"4201"
        }

		buildoptions {
			"/openmp:llvm"
		}

	filter "toolset:clang"
		disablewarnings {
			"unused-parameter",
		}

	filter "toolset:gcc"
		disablewarnings {
			"unused-parameter",
			"missing-field-initializers",
		}

    filter "action:vs*"
        linkoptions { "/ignore:4099" }
		buildoptions {
			"/Zc:preprocessor"
		}

group "ThirdParty"
include "ThirdParty/"
group ""

include "Yuki/"
include "Sandbox/"

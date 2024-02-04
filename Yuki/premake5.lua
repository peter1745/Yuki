local GRDK = os.getenv("GRDKLatest");

project "Yuki"
    kind "StaticLib"

	warnings "Extra"

	pchheader "YukiPCH.hpp"
	pchsource "Source/YukiPCH.cpp"

    files {
        "Source/YukiPCH.cpp",
        "Source/YukiPCH.hpp",

        "Source/Engine/**.cpp",
        "Source/Engine/**.hpp",
    }

	externalincludedirs {
		"Source/",
        "../ThirdParty/spdlog/include",
		"../ThirdParty/Aura/Aura/Include/",
		"../ThirdParty/rtmcpp/Include/",
		"../ThirdParty/rtmcpp/rtm/includes/",
		"../ThirdParty/stb/",
	}

	forceincludes {
		"YukiPCH.hpp"
	}

    defines {
        "RTMCPP_EXPORT=",
        "SPDLOG_USE_STD_FORMAT"
    }

    filter { "system:windows" }
		defines {
			"YUKI_PLATFORM_WINDOWS"
		}

        files {
            "Source/Platform/Windows/**.cpp",
            "Source/Platform/Windows/**.hpp",
        }

		externalincludedirs {
			GRDK .. "/GameKit/Include/",
		}

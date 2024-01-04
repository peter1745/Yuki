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
        "../ThirdParty/spdlog/include"
	}

	forceincludes {
		"YukiPCH.hpp"
	}

    defines {
        "SPDLOG_USE_STD_FORMAT"
    }

    filter { "system:windows" }
        files {
            "Source/Platform/Windows/**.cpp",
            "Source/Platform/Windows/**.hpp",
        }

		externalincludedirs {
			GRDK .. "/GameKit/Include/",
		}

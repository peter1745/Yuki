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
		"Source/"
	}

	forceincludes {
		"YukiPCH.hpp"
	}

    filter { "system:windows" }
        files {
            "Source/Platform/Windows/**.cpp",
            "Source/Platform/Windows/**.hpp",
        }

		externalincludedirs {
			GRDK .. "/GameKit/Include/",
		}

local GRDK = os.getenv("GRDKLatest");

project "EngineTester"
	kind "ConsoleApp"

	warnings "Extra"

	files {
		"Source/**.cpp",
		"Source/**.hpp",
	}

	externalincludedirs {
		"../Yuki/Source/",
		"../ThirdParty/Aura/Aura/Include/"
	}

	links {
		"Yuki",
        "Yuki-Vulkan"
	}

	filter { "system:windows" }
		defines {
			"YUKI_PLATFORM_WINDOWS"
		}

		libdirs {
			GRDK .. "/GameKit/Lib/amd64/",
			"../ThirdParty/DXC/lib/x64/"
		}

		links {
			"GameInput",
			"xgameruntime",
		}

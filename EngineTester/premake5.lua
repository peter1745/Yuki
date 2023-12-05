local GRDK = os.getenv("GRDKLatest");

project "EngineTester"
	kind "ConsoleApp"

	warnings "Extra"

	files {
		"Source/**.ixx",
	}

	links {
		"Yuki",
        "Yuki-D3D12",
        "Aura",
	}

	filter { "files:**.ixx" }
		compileas "Module"

	filter { "system:windows" }
		libdirs {
			GRDK .. "/GameKit/Lib/amd64/",
			"../ThirdParty/DXC/lib/x64/"
		}

		links {
			"GameInput",
			"xgameruntime",
            "D3d12",
            "DXGI",
			"dxcompiler"
		}

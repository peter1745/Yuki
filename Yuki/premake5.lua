local GRDK = os.getenv("GRDKLatest");

project "Yuki"
    kind "StaticLib"

	warnings "Extra"

    files {
        "Source/Engine/**.ixx",
    }

	links {
	    "Aura"
	}

	filter { "files:**.ixx" }
        compileas "Module"

    filter { "system:windows" }
        files {
            "Source/Platform/Windows/**.ixx",
        }

		externalincludedirs {
			GRDK .. "/GameKit/Include/",
		}

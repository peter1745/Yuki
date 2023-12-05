local GRDK = os.getenv("GRDKLatest");

project "Yuki-D3D12"
    kind "StaticLib"

	warnings "Extra"

    files {
        "Source/**.ixx",
    }

	externalincludedirs {
        "../ThirdParty/AgilitySDK/include/",
        "../ThirdParty/DXC/inc/",
	}

    links {
        "Yuki",
        "Aura",
    }

    postbuildcommands {
        '{COPYDIR} "../ThirdParty/AgilitySDK/bin/x64/" "%{wks.location}Build/Bin/%{cfg.buildcfg}/D3D12/"',
        '{COPYDIR} "../ThirdParty/DXC/bin/x64/" "%{wks.location}Build/Bin/%{cfg.buildcfg}/"',
    }

	filter { "files:**.ixx" }
        compileas "Module"

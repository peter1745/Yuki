project "Yuki-D3D12"
    kind "StaticLib"
	warnings "Extra"

    files {
        "Source/**.cpp",
        "Source/**.hpp"
    }

    externalincludedirs {
        "../Yuki/Source/",
        "../ThirdParty/AgilitySDK/include/"
    }

    postbuildcommands {
        "{COPYDIR} ../ThirdParty/AgilitySDK/bin/x64/ %{cfg.targetdir}/D3D12/"
    }
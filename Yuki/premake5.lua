project "Yuki"
    kind "StaticLib"

	warnings "Extra"
  
    pchheader "YukiPCH.hpp"
    pchsource "Source/YukiPCH.cpp"
    forceincludes { "YukiPCH.hpp" }

    files {
        "Source/YukiPCH.cpp",
        "Source/YukiPCH.hpp",

        "Source/Core/**.cpp",
        "Source/Core/**.hpp",

        "Include/**.hpp"
    }

    includedirs {
        "Source/",
        "Include/Yuki/"
    }

    externalincludedirs {
        "../ThirdParty/spdlog/include/"
    }

    defines { "SPDLOG_COMPILED_LIB" }

    filter { "system:windows" }
        defines { "YUKI_PLATFORM_WINDOWS" }

        files {
            "Source/Platform/**.cpp",
            "Source/Platform/**.hpp",
        }

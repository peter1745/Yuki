project "Sandbox"
    kind "ConsoleApp"

    files { "Source/**.cpp" }

	warnings "Extra"
   
    externalincludedirs {
        "../Yuki/Include/",
        
        "../ThirdParty/spdlog/include/",
        "../ThirdParty/ankerl/include/"
    }

    links {
        "Yuki",
        "spdlog"
    }

    defines { "SPDLOG_COMPILED_LIB" }

    filter { "system:windows" }
        defines { "YUKI_PLATFORM_WINDOWS" }


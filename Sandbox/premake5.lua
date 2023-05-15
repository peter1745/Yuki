project "Sandbox"
    kind "ConsoleApp"

    files { "Source/**.cpp" }

	warnings "Extra"
   
    externalincludedirs {
        "../Yuki/Include/",
        
        "../ThirdParty/spdlog/include/"
    }

    links {
        "Yuki",
        "spdlog"
    }

    defines { "SPDLOG_COMPILED_LIB" }


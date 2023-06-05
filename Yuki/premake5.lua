local VulkanSDKPath = os.getenv("VULKAN_SDK") .. "/include"

project "Yuki"
    kind "StaticLib"

	warnings "Extra"
  
    pchheader "YukiPCH.hpp"
    pchsource "Source/YukiPCH.cpp"
    forceincludes { "YukiPCH.hpp" }

    files {
        "Source/YukiPCH.cpp",
        "Source/YukiPCH.hpp",

        "Source/Engine/**.cpp",
        "Source/Engine/**.hpp",

        "Include/**.hpp"
    }

    includedirs {
        "Source/",
        "Include/",
        "Include/Yuki/",
    }

    externalincludedirs {
        "../ThirdParty/spdlog/include/",
        "../ThirdParty/volk/Include/",
        "../ThirdParty/ankerl/include/",

        VulkanSDKPath,
    }

    defines { "SPDLOG_COMPILED_LIB" }

    filter { "system:windows" }
        defines { "YUKI_PLATFORM_WINDOWS" }

        files {
            "Source/Platform/Windows/**.cpp",
            "Source/Platform/Windows/**.hpp",
        }
    
	filter { "system:linux" }
        defines { "YUKI_PLATFORM_LINUX" }

        files {
            "Source/Platform/Linux/**.cpp",
			"Source/Platform/Linux/**.hpp",
		}


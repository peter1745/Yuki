local VulkanSDKPath = os.getenv("VULKAN_SDK") .. "/Include/"

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
        "Include/Yuki/"
    }

    externalincludedirs {
        "../ThirdParty/spdlog/include/",
        "../ThirdParty/volk/Include/",

        VulkanSDKPath,
    }

    defines { "SPDLOG_COMPILED_LIB" }

    filter { "system:windows" }
        defines { "YUKI_PLATFORM_WINDOWS" }

        files {
            "Source/Platform/**.cpp",
            "Source/Platform/**.hpp",
        }

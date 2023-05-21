local VulkanSDKPath = os.getenv("VULKAN_SDK")

project "Sandbox"
    kind "ConsoleApp"

    files { "Source/**.cpp" }

	warnings "Extra"
   
    defines { "SPDLOG_COMPILED_LIB" }
   
    externalincludedirs {
        "../Yuki/Include/",
        
        "../ThirdParty/spdlog/include/",
        "../ThirdParty/ankerl/include/"
    }

    libdirs {
        VulkanSDKPath .. "/Lib/"
    }

    links {
        "Yuki",
        "spdlog"
    }

    filter { "configurations:Release" }
        links { "shaderc_combined" }

    filter { "configurations:Debug or configurations:RelWithDebug" }
        links { "shaderc_combinedd" }


    filter { "system:windows" }
        defines { "YUKI_PLATFORM_WINDOWS" }


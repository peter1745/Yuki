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
        VulkanSDKPath .. "/lib/"
    }

    links {
        "Yuki",
        "spdlog",
		"fastgltf",
		"simdjson",
    }

    filter { "configurations:Debug" }
        defines { "YUKI_CONFIG_DEBUG" }

    filter { "configurations:RelWithDebug" }
        defines { "YUKI_CONFIG_REL_WITH_DEBUG" }

    filter { "configurations:Release" }
        defines { "YUKI_CONFIG_RELEASE" }

    filter { "configurations:Release or system:linux" }
        links { "shaderc_combined" }

    filter { "system:windows", "configurations:Debug or configurations:RelWithDebug" }
        links { "shaderc_combinedd" }

    filter { "system:windows" }
        defines { "YUKI_PLATFORM_WINDOWS" }

    filter { "system:linux" }
        defines { "YUKI_PLATFORM_LINUX" }

		links {
			"xcb",
			"X11"
		}

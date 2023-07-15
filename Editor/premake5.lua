local VulkanSDKPath = os.getenv("VULKAN_SDK")

project "Editor"
    kind "ConsoleApp"

    files { "Source/**.cpp" }

	warnings "Extra"
   
    defines {
		"SPDLOG_COMPILED_LIB",
		"FASTNOISE_STATIC_LIB"
	}
   
    externalincludedirs {
        "../Yuki/Include/",
        
        "../ThirdParty/spdlog/include/",
        "../ThirdParty/ankerl/include/",
        "../ThirdParty/NFD-Extended/src/include/",
        "../ThirdParty/FastNoise2/include/",

        "../ThirdParty/",
    }

    libdirs {
        VulkanSDKPath .. "/lib/"
    }

    links {
        "Yuki",
        "spdlog",
		"fastgltf",
		"simdjson",
        "stb_image",
        "DearImGui",
		"flecs",
		"JoltPhysics",
		"NFD-Extended",
		"FastNoise",
    }

    filter { "configurations:Debug" }
        defines { "YUKI_CONFIG_DEBUG" }
		
		libdirs {
			"../ThirdParty/FastNoise2/lib/Debug/"
		}

		links {
			"glslangd",
			"glslang-default-resource-limitsd"
		}

    filter { "configurations:RelWithDebug" }
        defines { "YUKI_CONFIG_REL_WITH_DEBUG" }

		libdirs {
			"../ThirdParty/FastNoise2/lib/Debug/"
		}

		links {
			"glslangd",
			"glslang-default-resource-limitsd"
		}

    filter { "configurations:Release" }
        defines { "YUKI_CONFIG_RELEASE" }

		libdirs {
			"../ThirdParty/FastNoise2/lib/Release/"
		}

		links {
			"glslang",
			"glslang-default-resource-limits"
		}

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

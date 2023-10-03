local VulkanSDKPath = os.getenv("VULKAN_SDK")

project "EngineTester"
	kind "ConsoleApp"

	warnings "Extra"

	files {
		"Source/**.cpp",
		"Source/**.hpp",
	}

	includedirs {
		"Source/",
	}

	externalincludedirs { "../Yuki/Source/" }

	libdirs {
        VulkanSDKPath .. "/lib/"
    }

	links {
		"Yuki",
		"volk",
		"spdlog"
	}

	defines { "SPDLOG_COMPILED_LIB" }

	filter { "configurations:Debug" }
		defines { "YUKI_CONFIG_DEBUG" }

	filter { "configurations:RelWithDebug" }
		defines { "YUKI_CONFIG_REL_WITH_DEBUG" }

	filter { "configurations:Release" }
		defines { "YUKI_CONFIG_RELEASE" }

	filter { "system:windows" }
		defines { "YUKI_PLATFORM_WINDOWS" }

	filter { "system:linux" }
		defines { "YUKI_PLATFORM_LINUX" }


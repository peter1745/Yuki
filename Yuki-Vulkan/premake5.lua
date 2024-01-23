local VulkanIncludeDir = os.getenv("VULKAN_SDK") .. "/include"

project "Yuki-Vulkan"
    kind "StaticLib"
	warnings "Extra"

    files {
        "Source/**.cpp",
        "Source/**.hpp",

		"../ThirdParty/volk/volk.c"
    }

    externalincludedirs {
        "../Yuki/Source/",
		"../ThirdParty/volk/",
		"../ThirdParty/Aura/Aura/Include/",

		VulkanIncludeDir,
    }

	defines {
		"VK_NO_PROTOTYPES"
	}

	filter { "system:windows" }
		defines {
			"YUKI_PLATFORM_WINDOWS",
			"VK_USE_PLATFORM_WIN32_KHR"
		}

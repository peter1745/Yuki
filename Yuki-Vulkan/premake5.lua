local VulkanLib = os.getenv("VULKAN_SDK") .. "/Lib"
local VulkanIncludeDir = os.getenv("VULKAN_SDK") .. "/Include"

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

	libdirs {
		VulkanLib
	}

	filter { "configurations:Debug or configurations:RelWithDebug" }
		links {
			"glslangd",
			"glslang-default-resource-limitsd",
			"OSDependentd",
			"MachineIndependentd",
			"GenericCodeGend",
			"SPIRVd",
			"SPIRV-Toolsd",
			"SPIRV-Tools-diffd",
			"SPIRV-Tools-linkd",
			"SPIRV-Tools-lintd",
			"SPIRV-Tools-optd",
			"SPIRV-Tools-reduced",
		}

	filter { "configurations:Release" }
		links {
			"glslang",
			"glslang-default-resource-limitsd",
			"OSDependentd",
			"MachineIndependentd",
			"GenericCodeGend",
			"SPIRVd",
			"SPIRV-Toolsd",
			"SPIRV-Tools-diffd",
			"SPIRV-Tools-linkd",
			"SPIRV-Tools-lintd",
			"SPIRV-Tools-optd",
			"SPIRV-Tools-reduced",
		}

	filter { "system:windows" }
		defines {
			"YUKI_PLATFORM_WINDOWS",
			"VK_USE_PLATFORM_WIN32_KHR"
		}

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

	externalincludedirs {
        "../Yuki/Source/",
        "../ThirdParty/glm/"
    }

	libdirs {
        VulkanSDKPath .. "/Lib/"
    }

	links {
		"Yuki",
		"volk",
		"spdlog",
        "simdjson",
        "fastgltf",
		"stb_image",
	}

	defines { "SPDLOG_COMPILED_LIB" }

	filter { "configurations:Debug" }
		defines { "YUKI_CONFIG_DEBUG" }

		links {
			"glslangd",
			"glslang-default-resource-limitsd",
			"OGLCompilerd",
			"OSDependentd",
			"MachineIndependentd",
			"GenericCodeGend",
			"SPIRVd",
			"SPVRemapperd",
			"SPIRV-Toolsd",
			"SPIRV-Tools-diffd",
			"SPIRV-Tools-linkd",
			"SPIRV-Tools-lintd",
			"SPIRV-Tools-optd",
			"SPIRV-Tools-reduced",
		}

	filter { "configurations:RelWithDebug" }
		defines { "YUKI_CONFIG_REL_WITH_DEBUG" }

		links {
			"glslangd",
			"glslang-default-resource-limitsd",
			"OGLCompilerd",
			"OSDependentd",
			"MachineIndependentd",
			"GenericCodeGend",
			"SPIRVd",
			"SPVRemapperd",
			"SPIRV-Toolsd",
			"SPIRV-Tools-diffd",
			"SPIRV-Tools-linkd",
			"SPIRV-Tools-lintd",
			"SPIRV-Tools-optd",
			"SPIRV-Tools-reduced",
		}

	filter { "configurations:Release" }
		defines { "YUKI_CONFIG_RELEASE" }

		links {
			"glslang",
			"glslang-default-resource-limitsd",
			"OGLCompiler",
			"OSDependent",
			"MachineIndependent",
			"GenericCodeGen",
			"SPIRV",
			"SPVRemapper",
			"SPIRV-Tools",
			"SPIRV-Tools-diff",
			"SPIRV-Tools-link",
			"SPIRV-Tools-lint",
			"SPIRV-Tools-opt",
			"SPIRV-Tools-reduce",
		}

	filter { "system:windows" }
		defines { "YUKI_PLATFORM_WINDOWS" }

	filter { "system:linux" }
		defines { "YUKI_PLATFORM_LINUX" }


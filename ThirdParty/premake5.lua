project "spdlog"
    kind "StaticLib"

    files {
        "spdlog/src/**.cpp",
        "spdlog/src/**.cpp",
    }

    includedirs { "spdlog/include/" }

    defines { "SPDLOG_COMPILED_LIB" }

local VulkanSDKPath = os.getenv("VULKAN_SDK") .. "/include"

project "volk"
	kind "StaticLib"

	files {
		"./volk/Include/volk/volk.c"
	}

	externalincludedirs {
		VulkanSDKPath
	}

    filter { "system:windows" }
        defines { "VK_USE_PLATFORM_WIN32_KHR" }

    filter { "system:linux" }
        defines { "VK_USE_PLATFORM_XCB_KHR" }

project "simdjson"
	kind "StaticLib"

	defines {
		"SIMDJSON_DISABLE_DEPRECATED_API=1"
	}

	files {

		"simdjson/src/simdjson.cpp"
	}

	includedirs {
		"simdjson/include/"
	}

project "fastgltf"
	kind "StaticLib"

	files {
		"fastgltf/src/fastgltf.cpp",
		"fastgltf/src/base64.cpp"
	}

	includedirs {
		"fastgltf/include/",
		"simdjson/include/"
	}

project "stb_image"
	kind "StaticLib"

	files {
		"stb_image/src/stb_image.cpp"
	}

	includedirs {
		"stb_image/include/stb_image/"
	}

--[[local VulkanSDKPath = os.getenv("VULKAN_SDK") .. "/include"

project "DearImGui"
	kind "StaticLib"

	files {
		"imgui/imgui_draw.cpp",
		"imgui/imgui_tables.cpp",
		"imgui/imgui.cpp",
		"imgui/imgui_widgets.cpp",

		"imgui/backends/imgui_impl_vulkan.cpp",
		"imgui/misc/cpp/imgui_stdlib.cpp",
	}

	includedirs {
		"imgui/"
	}

	externalincludedirs {
		VulkanSDKPath
	}

	defines {
		"IMGUI_IMPL_VULKAN_NO_PROTOTYPES"
	}

	filter { "system:windows" }
		files {
			"imgui/backends/imgui_impl_win32.cpp"
		}

project "flecs"
	kind "StaticLib"
	language "C"
	cdialect "C17"

	files {
		"flecs/flecs.c",
	}

project "JoltPhysics"
	kind "StaticLib"
	
	files {
		"JoltPhysics/Jolt/**.cpp",
	}

	includedirs {
		"JoltPhysics/"
	}

project "NFD-Extended"
	kind "StaticLib"

	includedirs {
		"NFD-Extended/src/include/"
	}

	filter { "system:windows" }
		files {
			"NFD-Extended/src/nfd_win.cpp"
		}
]]--
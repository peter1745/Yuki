project "spdlog"
    kind "StaticLib"

    files {
        "spdlog/src/**.cpp",
        "spdlog/src/**.cpp",
    }

    includedirs { "spdlog/include/" }

    defines { "SPDLOG_COMPILED_LIB" }

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
	
project "DearImGui"
	kind "StaticLib"

	files {
		"imgui/imgui_draw.cpp",
		"imgui/imgui_tables.cpp",
		"imgui/imgui.cpp",
		"imgui/imgui_widgets.cpp",

		"imgui/misc/cpp/imgui_stdlib.cpp",
	}

	includedirs {
		"imgui/"
	}

	filter { "system:windows" }
		files {
			"imgui/backends/imgui_impl_win32.cpp"
		}


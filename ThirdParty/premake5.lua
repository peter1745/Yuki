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

	files {
		"simdjson/src/simdjson.cpp"
	}

	includedirs {
		"simdjson/include/simdjson/"
	}

project "fastgltf"
	kind "StaticLib"

	files {
		"fastgltf/src/fastgltf.cpp",
		"fastgltf/src/base64.cpp"
	}

	includedirs {
		"fastgltf/include/",
		"simdjson/include/simdjson/"
	}


project "spdlog"
	location "spdlog/"
    kind "StaticLib"

    files {
        "spdlog/src/*.cpp"
    }

    includedirs {
        "spdlog/include/"
    }

    defines {
        "SPDLOG_COMPILED_LIB",
        "SPDLOG_USE_STD_FORMAT"
    }

project "rtmcpp"
    location "rtmcpp/"
	kind "None"

	files {
		"rtmcpp/**.hpp"
	}

	externalincludedirs {
		"rtmcpp/rtm/includes/"
	}

    defines {
        "RTMCPP_EXPORT="
    }

project "Aura"
	location "Aura/"
    kind "StaticLib"

    files {
        "Aura/Aura/Include/**.hpp"
    }

    includedirs {
        "Aura/Aura/Include/"
    }

project "simdjson"
    location "simdjson/"
    kind "StaticLib"

    files {
        "simdjson/src/simdjson.cpp",
    }

    includedirs {
        "simdjson/src/",
        "simdjson/include/",
        "simdjson/include/simdjson/",
    }

project "fastgltf"
    location "fastgltf/"
    kind "StaticLib"

    files {
        "fastgltf/src/**.cpp",
    }

    includedirs {
        "fastgltf/include/"
    }

    externalincludedirs {
        "simdjson/include/"
    }

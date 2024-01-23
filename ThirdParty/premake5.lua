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

project "Aura"
	location "Aura/"
    kind "StaticLib"

    files {
        "Aura/Aura/Include/**.hpp"
    }

    includedirs {
        "Aura/Aura/Include/"
    }
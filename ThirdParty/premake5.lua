project "spdlog"
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

local VulkanSDKPath = os.getenv("VULKAN_SDK") .. "/include"

project "Yuki"
    kind "StaticLib"

	warnings "Extra"
  
    pchheader "YukiPCH.hpp"
    pchsource "Source/YukiPCH.cpp"
	forceincludes { "YukiPCH.hpp" }

    files {
        "Source/YukiPCH.cpp",
        "Source/YukiPCH.hpp",

        "Source/Engine/**.cpp",
        "Source/Engine/**.hpp",

		"Source/VulkanRHI/**.cpp",
		"Source/VulkanRHI/**.hpp",
    }

    includedirs {
        "Source/",
    }

    externalincludedirs {
        "../ThirdParty/spdlog/include/",
        "../ThirdParty/volk/Include/",
        "../ThirdParty/ankerl/include/",
        "../ThirdParty/fastgltf/include/",
        "../ThirdParty/stb_image/include/",
        "../ThirdParty/imgui/",
        "../ThirdParty/nlohmann/json/single_include/",
        "../ThirdParty/simdjson/include/",
		"../ThirdParty/volk/Include/",
        "../ThirdParty/glm/",

        VulkanSDKPath,
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

        files {
            "Source/Platform/Windows/**.cpp",
            "Source/Platform/Windows/**.hpp",
        }
    
	filter { "system:linux" }
        defines { "YUKI_PLATFORM_LINUX" }

        files {
            "Source/Platform/Linux/**.cpp",
			"Source/Platform/Linux/**.hpp",
		}

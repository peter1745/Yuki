local function getParentPath(path)
    pattern1 = "^(.+)//"
    pattern2 = "^(.+)\\"

    if (string.match(path,pattern1) == nil) then
        return string.match(path,pattern2)
    else
        return string.match(path,pattern1)
    end
end

local VulkanSDKPath = getParentPath(os.getenv("VULKAN_SDK")) .. "/1.3.246.1/Include"

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

        "Include/**.hpp"
    }

    includedirs {
        "Source/",
        "Include/Yuki/"
    }

    externalincludedirs {
        "../ThirdParty/spdlog/include/",
        "../ThirdParty/volk/Include/",
        "../ThirdParty/ankerl/include/",

        VulkanSDKPath,
    }

    defines { "SPDLOG_COMPILED_LIB" }

    filter { "system:windows" }
        defines { "YUKI_PLATFORM_WINDOWS" }

        files {
            "Source/Platform/**.cpp",
            "Source/Platform/**.hpp",
        }

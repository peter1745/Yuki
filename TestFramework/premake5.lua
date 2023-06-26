project "TestFramework"
    kind "ConsoleApp"

    files { "Source/**.cpp" }

	warnings "Extra"
   
    externalincludedirs {
        "../ThirdParty/",
    }

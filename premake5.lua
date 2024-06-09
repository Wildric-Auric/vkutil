workspace "VKIFR"
	configurations {"Release", "Debug"}
	architecture "x64"
	language "C++"
    cppdialect "C++11"
	targetdir "build/bin/%{prj.name}/%{cfg.buildcfg}"
    objdir    "build/bin/objs"
    characterset("MBCS")
    buildoptions { "/EHsc" }
	location "build"
	project "VKIFR"
	kind "ConsoleApp"
	includedirs {
		"vendor/stb/",
		"C:/VulkanSDK/1.3.268.0/include/",
		"src/",
        "vendor/tiny/",
        "vendor/"
	}
	files {
		"src/*.cpp",
		"src/*.h",
		"src/*.hpp",
		"premake5.lua",
		"src/*.vert",
		"src/*.frag",
        "vendor/nwin/*.cpp",
        "vendor/nwin/*.h"
	}
	links {
        "dwmapi.lib",
	    "C:/VulkanSDK/1.3.268.0/Lib/vulkan-1.lib"
	}
	filter "configurations:Release"
		optimize "On"
	filter "configurations:Debug"
		symbols "On"

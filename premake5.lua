vkpath = os.getenv("VK_SDK_PATH")
print(vkpath)
workspace "vkUtilExample"
	configurations {"Release", "Debug"}
	architecture "x64"
	language "C++"
    cppdialect "C++11"
	targetdir "build/bin/%{prj.name}/%{cfg.buildcfg}"
    objdir    "build/bin/objs"
    characterset("MBCS")
    buildoptions { "/EHsc" }
	location "build"
	project "vkUtilExample"
	kind "ConsoleApp"
	includedirs {
		"vendor/stb/",
		"%{vkpath}/include/",
		"src/",
        "vendor/LWmath/",
        "vendor/"
	}
	files {
		"src/**.cpp",
		"src/**.h",
		"src/**.hpp",
		"premake5.lua",
		"src/**",
		"src/**",
        "vendor/nwin/*.cpp",
        "vendor/nwin/*.h"
	}
	links {
        "dwmapi.lib",
		"%{vkpath}/Lib/vulkan-1.lib"
	}
	filter "configurations:Release"
		optimize "On"
	filter "configurations:Debug"
		symbols "On"

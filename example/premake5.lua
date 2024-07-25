project "example"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	files {
		"src/**.cpp",
		"src/**.hpp",
	}

	includedirs {
		"../result/include",
	}

	links {
		"result",
	}

	targetdir "bin"
	objdir "obj/%{cfg.buildcfg}"

	filter "configurations:Debug"
		ignoredefaultlibraries { "MSVCRT" }
		targetname "%{prj.name}d"
		optimize "off"
		symbols "on"

	filter "configurations:Release"
		optimize "on"
		symbols "off"

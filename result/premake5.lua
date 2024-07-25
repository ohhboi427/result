project "result"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	files {
		"src/**.cpp",
		"include/**.hpp",
	}

	includedirs {
		"include",
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

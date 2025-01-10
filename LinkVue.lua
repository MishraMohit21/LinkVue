workspace "LinkVue"
	architecture "x64"
	startproject "LinkVue"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to the root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "LinkVue/vendor/GLFW/include"
IncludeDir["Glad"] = "LinkVue/vendor/Glad/include"
IncludeDir["ImGui"] = "LinkVue/vendor/imgui"
IncludeDir["GameNetworkingSockets"] = "LinkVue/vendor/GameNetworkingSockets/include"

group "Dependencies"
	include "LinkVue/vendor/GLFW"
	include "LinkVue/vendor/Glad"
	include "LinkVue/vendor/imgui"
group ""

-- Shared settings for host and client modules
function setup_project(project_name, project_dir)
	project(project_name)
		location(project_dir)
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"
		staticruntime "off"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"./" .. project_dir .. "/Source/**.h",
			"./" .. project_dir .. "/Source/**.cpp"
		}

		includedirs
		{
			"Source",
			"LinkVue/Source",
			"%{IncludeDir.GameNetworkingSockets}"
		}

		links 
		{ 
			"Ws2_32.lib" -- For networking on Windows
		}

		filter "system:windows"
			systemversion "latest"
			defines { "LV_PLATFORM_WINDOWS" }

		filter "system:linux"
			defines { "LV_PLATFORM_LINUX" }

		filter "configurations:Debug"
			defines { "LV_DEBUG" }
			runtime "Debug"
			symbols "On"

		filter "configurations:Release"
			defines { "LV_RELEASE" }
			runtime "Release"
			optimize "On"
			symbols "On"

		filter "configurations:Dist"
			defines { "LV_DIST" }
			runtime "Release"
			optimize "On"
			symbols "Off"
end

setup_project("LinkVue-Host", "LinkVue-Host")
setup_project("LinkVue-Client", "LinkVue-Client")

project "LinkVue"
	location "LinkVue"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"./LinkVue/Source/**.h",
		"./LinkVue/Source/**.cpp"
	}

	includedirs
	{
		"$(SolutionDir)Source",
		"$(SolutionDir)%{IncludeDir.GLFW}",
		"$(SolutionDir)%{IncludeDir.Glad}",
		"$(SolutionDir)%{IncludeDir.ImGui}",
		"$(SolutionDir)%{IncludeDir.GameNetworkingSockets}"
	}

	links 
	{
		"GLFW",
		"Glad",
		"ImGui",
		"LinkVue-Host",
		"LinkVue-Client"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "LV_PLATFORM_WINDOWS" }

	filter "system:linux"
		defines { "LV_PLATFORM_LINUX" }

	filter "configurations:Debug"
		defines { "LV_DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines { "LV_RELEASE" }
		runtime "Release"
		optimize "On"
		symbols "On"

	filter "configurations:Dist"
		defines { "LV_DIST" }
		runtime "Release"
		optimize "On"
		symbols "Off"

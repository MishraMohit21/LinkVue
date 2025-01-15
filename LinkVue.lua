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
IncludeDir["yaml_cpp"] = "vendor/yaml-cpp/include"
IncludeDir["ImGui"] = "LinkVue/vendor/imgui"
IncludeDir["GameNetworkingSockets"] = "LinkVue/vendor/GameNetworkingSockets/include"

group "Dependencies"
	include "LinkVue/vendor/GLFW"
	include "LinkVue/vendor/Glad"
	include "LinkVue/vendor/imgui"
group ""


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
		"$(ProjectDir)Source",
		"$(ProjectDir)vendor/spdlog/include",
		"$(ProjectDir)%{IncludeDir.yaml_cpp}",
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
	}

	filter "system:windows"
		systemversion "latest"
		defines { "LV_PLATFORM_WINDOWS" }

	filter "system:linux"
		defines { "LV_PLATFORM_LINUX" }

	filter { "system:windows", "configurations:Debug" }	
		links
		{
			"$(ProjectDir)vendor/GameNetworkingSockets/bin/Windows/Debug/GameNetworkingSockets.lib"
		}
  
	filter { "system:windows", "configurations:Release or configurations:Dist" }	
		links
		{
			"$(ProjectDir)vendor/GameNetworkingSockets/bin/Windows/Release/GameNetworkingSockets.lib"
		}

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

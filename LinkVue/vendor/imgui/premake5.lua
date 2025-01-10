project "ImGui"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",



		"examples/imgui_impl_glfw.h",
		"examples/imgui_impl_glfw.cpp",
		"examples/imgui_impl_opengl3.h",
		"examples/imgui_impl_opengl3.cpp"
	}

	includedirs
    {
        ".",
        "$(SolutionDir)%{IncludeDir.GLFW}" , -- Adjust this path to where your GLFW headers are
        "$(SolutionDir)%{IncludeDir.Glad}"  -- Adjust this path to where your GLFW headers are
    }


	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		defines { "_DEBUG" }
		buildoptions { "/MDd" } 

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		buildoptions { "/MD" }

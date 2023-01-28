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
		
		"imgui_demo.cpp",
        "imgui_draw.cpp",
		
		"imgui_tables.cpp",
        "imgui_internal.h",
        "imgui_widgets.cpp",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
        
		"imgui_impl_glfw.cpp",
		"imgui_impl_glfw.h",
		
		"imgui_impl_opengl3.cpp",
		"imgui_impl_opengl3.h",
		"imgui_impl_opengl3_loader.h"
    }
    
	filter "system:windows"
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "On"
        
    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"
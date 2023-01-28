workspace "Hayase Renderer"
	architecture "x64"
	startproject "Hayase Renderer"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Libraries/GLFW/include"
IncludeDir["Glad"] = "Libraries/Glad/include"
IncludeDir["glm"] = "Libraries/glm"
IncludeDir["ImGui"] = "Libraries/imgui"
IncludeDir["stb"] = "Libraries/stb"
IncludeDir["tinygltf"] = "Libraries/tinygltf"
IncludeDir["tinyobj"] = "Libraries/tinyobjloader"

include "Libraries/GLFW"
include "Libraries/Glad"
include "Libraries/imgui"

project "Hayase Renderer"
	location "Hayase Renderer"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	pchheader "hyspch.h"
	pchsource "Hayase Renderer/Core/hyspch.cpp"
	
	files
	{
		"%{prj.name}/Core/**.h",
		"%{prj.name}/Core/**.cpp",
		"%{prj.name}/Core/**.hpp",
		"%{prj.name}/Events/**.h",
		"%{prj.name}/Events/**.cpp",
		"%{prj.name}/Editor/**.h",
		"%{prj.name}/Editor/**.cpp",
		"%{prj.name}/IO/**.h",
		"%{prj.name}/IO/**.cpp",
		"%{prj.name}/Rendering/**.h",
		"%{prj.name}/Rendering/**.cpp",
		"%{prj.name}/Rendering/**.hpp",
		"%{prj.name}/Scene Management/**.h",
		"%{prj.name}/Scene Management/**.cpp",
		"%{prj.name}/Utilities/**.h",
		"%{prj.name}/Utilities/**.cpp",
		
		"Libraries/glm/**.hpp",
		"Libraries/glm/**.inl",
		
		"Libraries/stb/**.h",
		
		"Libraries/tinygltf/**.h",
		"Libraries/tinygltf/**.hpp",
		
		"Libraries/tinyobjloader/**.h"
	}
	
	includedirs
	{
		"%{prj.name}/Core",
		"%{prj.name}/Events",
		"%{prj.name}/IO",
		"%{prj.name}/Libraries",
		"%{prj.name}/Rendering",
		"%{prj.name}/Scene Management",
		"%{prj.name}/Scene Management/Scenes",
		"%{prj.name}/Utilities",
		"%{prj.name}/Utilities/Layers",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.tinyobj}"
	}
	
	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
workspace "ARI-S"
	architecture "x64"
	startproject "ARI-S"
	
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
IncludeDir["entt"] = "Libraries/entt"
IncludeDir["stb"] = "Libraries/stb"
IncludeDir["tinygltf"] = "Libraries/tinygltf"
IncludeDir["tinyobj"] = "Libraries/tinyobjloader"
IncludeDir["YAML_CPP"] = "Libraries/yaml-cpp/include"

include "Libraries/GLFW"
include "Libraries/Glad"
include "Libraries/imgui"
include "Libraries/yaml-cpp"

project "ARI-S"
	location "ARI-S"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	pchheader "arpch.h"
	pchsource "ARI-S/Core/arpch.cpp"
	
	files
	{
		"%{prj.name}/Core/**.h",
		"%{prj.name}/Core/**.cpp",
		"%{prj.name}/Core/**.hpp",
		"%{prj.name}/ECS/**.h",
		"%{prj.name}/ECS/**.cpp",
		"%{prj.name}/ECS/**.hpp",
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
		"%{prj.name}/Utilities/**.hpp",
		
		"Libraries/glm/**.hpp",
		"Libraries/glm/**.inl",
		
		"Libraries/entt/**.hpp",
		
		"Libraries/stb/**.h",
		
		"Libraries/tinygltf/**.h",
		"Libraries/tinygltf/**.hpp",
		
		"Libraries/tinyobjloader/**.h"
	}
	
	includedirs
	{
		"%{prj.name}/Core",
		"%{prj.name}/ECS",
		"%{prj.name}/ECS/Components",
		"%{prj.name}/Events",
		"%{prj.name}/IO",
		"%{prj.name}/Libraries",
		"%{prj.name}/Rendering",
		"%{prj.name}/Rendering/Memory",
		"%{prj.name}/Rendering/Modeling",
		"%{prj.name}/Scene Management",
		"%{prj.name}/Utilities",
		"%{prj.name}/Utilities/Layers",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.tinyobj}",
		"%{IncludeDir.YAML_CPP}"
	}
	
	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"YAML-CPP",
		"opengl32.lib"
	}
	
	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
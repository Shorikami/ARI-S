project "YAML-CPP"
    kind "StaticLib"
    language "C++"
    
	files
	{
	    "src/**.h",
        "src/**.cpp",
	
        "include/**.h"
    }
	
	includedirs
	{
		"include"
	}
    
	filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"
        staticruntime "off"
       
    filter { "system:windows", "configurations:Release" }
        runtime "Release"
		optimize "on"
#version 430 core
layout (location = 0) out vec3 gPos;
layout (location = 1) out vec3 gNorm;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec2 gMetRough;
layout (location = 4) out float entID;
layout (location = 5) out float gDepth;

in vec3 outPos;
in vec3 outNorm;
in vec2 outTexCoord;
in float vEntityID;
in vec4 viewPos;

uniform int metRoughCombine;

uniform sampler2D diffTex1;
uniform sampler2D specTex1;
uniform sampler2D normTex1;
uniform sampler2D heightTex1;
uniform sampler2D metalRoughTex1;
uniform sampler2D metalTex1;
uniform sampler2D roughTex1;

void main()
{
	gPos = outPos;
	gNorm = normalize(outNorm);
	//gUVs = outTexCoord;
	gAlbedo = texture(diffTex1, outTexCoord).rgb;
	
	if (metRoughCombine == 1)
	{
		// g = metallic
		// b = roughness
		gMetRough = texture(metalRoughTex1, outTexCoord).gb;
	}
	else
	{
		vec3 metal = texture(metalTex1, outTexCoord).rrr;
		vec3 roughness = texture(roughTex1, outTexCoord).rrr;
		gMetRough = vec2(metal.r, roughness.r);
	}
	
	//gSpecular = texture(specTex, outTexCoord).rrr;
	entID = vEntityID;
	
	gDepth = gl_FragCoord.z;
}
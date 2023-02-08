#version 430 core

out vec4 fragColor;

layout (binding = 0) uniform sampler2D gPos;
layout (binding = 1) uniform sampler2D gNorm;
layout (binding = 2) uniform sampler2D gUVs;
layout (binding = 3) uniform sampler2D gAlbedo;
layout (binding = 4) uniform sampler2D gSpecular;
layout (binding = 5) uniform sampler2D gDepth;

uniform vec3 viewPos;
uniform int vWidth;
uniform int vHeight;

uniform vec3 lightDir;

vec3 LightCalc()
{
	vec2 fragUV = vec2(gl_FragCoord.x / vWidth, gl_FragCoord.y / vHeight);

	vec3 fragPos = texture(gPos, fragUV).rgb;
	vec3 norm = texture(gNorm, fragUV).rgb;
	vec2 uv = texture(gUVs, fragUV).rg;
	vec3 diff = texture(gAlbedo, fragUV).rgb;
	vec3 specTex = texture(gSpecular, fragUV).rgb;
	float spec = texture(gDepth, fragUV).r;

	// ambient
	vec3 amb = diff * 0.2f;
	
	// diffuse
	vec3 dir = normalize(-lightDir);
	float nDotL = max(dot(norm, dir), 0.0);
	vec3 finalDiff = nDotL * diff;
	
	// specular
	vec3 viewDir = normalize(viewPos - fragPos);
	
	vec3 reflectDir = reflect(dir, norm);
	float sp = pow(max(dot(viewDir, reflectDir), 0.0), 16.0f);
	vec3 finalSpec = sp * spec * specTex;
	
	return amb + finalDiff + finalSpec;
}

void main()
{
	vec2 uv = vec2(gl_FragCoord.x / vWidth, 
					gl_FragCoord.y / vHeight);
					
	vec3 localLight = LightCalc();
	
	fragColor = vec4(localLight, 1.0f);
}
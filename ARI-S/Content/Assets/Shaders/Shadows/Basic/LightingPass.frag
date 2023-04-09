#version 450 core

out vec4 fragColor;

layout (binding = 0) uniform sampler2D gPos;
layout (binding = 1) uniform sampler2D gNorm;
layout (binding = 2) uniform sampler2D gAlbedo;

uniform vec3 viewPos;
uniform int vWidth;
uniform int vHeight;

uniform vec3 lightDir;
uniform mat4 worldToLightMat;

uniform sampler2D uShadowMap;


vec3 LightCalc()
{
	vec2 fragUV = vec2(gl_FragCoord.x / vWidth, gl_FragCoord.y / vHeight);

	vec3 fragPos = texture(gPos, fragUV).rgb;
	vec3 norm = texture(gNorm, fragUV).rgb;
	vec3 diff = texture(gAlbedo, fragUV).rgb;

	// diffuse
	vec3 dir = normalize(lightDir - fragPos);
	float nDotL = max(dot(norm, dir), 0.0);
	vec3 finalDiff = nDotL * vec3(1.0f);
	
	// specular
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-dir, norm);
	
	vec3 H = normalize(dir + viewDir);
	float spec = pow(max(dot(norm, H), 0.0f), 64.0f);
	vec3 finalSpec = spec * vec3(1.0f);
	
	// shadows
	vec4 shadowCoord = worldToLightMat * vec4(fragPos, 1.0f);
	vec2 shadowIdx = shadowCoord.xy / shadowCoord.w;
	shadowIdx = shadowIdx * 0.5f + 0.5f;
	
	float lightDepth = texture(uShadowMap, shadowCoord.xy).r;
	float pixelDepth = shadowCoord.z;
	float shadow = pixelDepth > lightDepth ? 1.0f : 0.0f;
	
	return vec3(1 - shadow);
}

void main()
{
	vec3 localLight = LightCalc();
	
	fragColor = vec4(localLight, 1.0f);
}
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

float Shadow(vec4 v)
{
	vec3 coords = v.xyz / v.w;
	v = v * 0.5f + 0.5f;
	
	float closest = texture(uShadowMap, v.xy).r;
	float curr = v.z;
	
	float shadow = closest;
	
	if (curr > 1.0f)
		shadow = 0.0f;
		
	return shadow;
}

vec3 LightCalc()
{
	vec2 fragUV = vec2(gl_FragCoord.x / vWidth, gl_FragCoord.y / vHeight);

	vec3 fragPos = texture(gPos, fragUV).rgb;
	vec3 norm = texture(gNorm, fragUV).rgb;
	vec3 diff = texture(gAlbedo, fragUV).rgb;

	// diffuse
	vec3 dir = normalize(-lightDir);
	float nDotL = max(dot(norm, dir), 0.0f);
	vec3 finalDiff = nDotL * diff;
	
	// specular
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-dir, norm);
	
	vec3 H = normalize(dir + viewDir);
	float spec = pow(max(dot(norm, H), 0.0f), 64.0f);
	vec3 finalSpec = spec * vec3(1.0f);
	
	// shadows
	vec4 shadowCoord = worldToLightMat * vec4(fragPos, 1.0f);
	float shadow = Shadow(shadowCoord);
	
	return (1.0f - shadow) * diff;
}

void main()
{
	vec3 localLight = LightCalc();
	
	fragColor = vec4(localLight, 1.0f);
}
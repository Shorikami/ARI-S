#version 430 core

out vec4 fragColor;

in vec3 fragPos;
in vec3 fragNorm;

layout (binding = 0) uniform sampler2D gPos;
layout (binding = 1) uniform sampler2D gNorm;
layout (binding = 2) uniform sampler2D gUVs;
layout (binding = 3) uniform sampler2D gAlbedo;
layout (binding = 4) uniform sampler2D gSpecular;
layout (binding = 5) uniform sampler2D gDepth;

uniform vec3 pos;
uniform vec4 color;

uniform vec3 eyePos;

uniform float range;
uniform float intensity;

uniform int vWidth;
uniform int vHeight;

vec3 LightCalc()
{
	vec2 fragUV = vec2(gl_FragCoord.x / vWidth, gl_FragCoord.y / vHeight);
	
	vec3 gFragPos = texture(gPos, fragUV).rgb;
	vec3 norm = texture(gNorm, fragUV).rgb;
	vec3 diffTex = texture(gAlbedo, fragUV).rgb;
	vec3 specTex = texture(gSpecular, fragUV).rgb;

	vec3 L = pos - gFragPos;
	float dist = distance(gFragPos, pos);
	L /= dist;

	// ambient
	//vec3 amb = (diff * 0.2f);
	
	// diffuse
	float nDotL = max(dot(norm, L), 0.0);
	vec3 finalDiff = intensity * nDotL * diffTex * color.xyz;

	// specular
	vec3 viewDir = normalize(eyePos - fragPos);
	
	vec3 reflectDir = reflect(L, norm);
	float sp = pow(max(dot(viewDir, reflectDir), 0.0), 16.0f);
	vec3 finalSpec = intensity * color.xyz * sp * specTex;

	// attenuation
	float att = ((1.0f / (dist * dist)) - (1.0f / pow(0.08f * range, 2)));
	
	return att * (finalDiff + finalSpec);
}

void main()
{
	vec2 fragUV = vec2(gl_FragCoord.x / vWidth, 
	gl_FragCoord.y / vHeight);

	vec3 gFragPos = texture(gPos, fragUV).rgb;
	vec3 L = pos - gFragPos;
	float dist = length(L);
	
    // arbitrary scale for the distance check because the
    // influence sphere scales based on the original radius
    // (in this case it's 0.08f)
	//if (dist > 0.08f * range)
	//{
	//	discard;
	//}

	fragColor = vec4(vec3(0.5f), 1.0f);
}
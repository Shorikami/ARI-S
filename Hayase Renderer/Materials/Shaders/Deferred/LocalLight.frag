#version 410 core

out vec4 fragColor;

in vec3 fragPos;
in vec3 fragNorm;

layout (binding = 0) uniform sampler2D gPos;
layout (binding = 1) uniform sampler2D gNorm;
layout (binding = 2) uniform sampler2D gUVs;
layout (binding = 3) uniform sampler2D gAlbedo;
layout (binding = 4) uniform sampler2D gSpecular;
layout (binding = 5) uniform sampler2D gDepth;

layout(std140, binding = 2) uniform LocalLight
{
  vec4 pos; // xyz = position, w = range
  vec4 color;
  vec4 options; // x = intensity, y = cutoff, z = max range
};

uniform vec3 eyePos;
uniform int attCalc;

vec3 LightCalc()
{
	vec3 gFragPos = texture(gPos, gl_FragCoord.xy / vec2(1600, 900)).rgb;
	vec3 norm = texture(gNorm, gl_FragCoord.xy / vec2(1600, 900)).rgb;
	vec3 diffTex = texture(gAlbedo, gl_FragCoord.xy / vec2(1600, 900)).rgb;
	vec3 specTex = texture(gSpecular, gl_FragCoord.xy / vec2(1600, 900)).rgb;

	vec3 L = pos.xyz - gFragPos;
	float dist = length(L);
	L /= dist;

	// ambient
	//vec3 amb = (diff * 0.2f);
	
	// diffuse
	vec3 dir = normalize(L);
	float nDotL = max(dot(norm, dir), 0.0);
	vec3 finalDiff = options.x * nDotL * diffTex * color.xyz;
	
	// specular
	// correct?
	//vec3 eyePosition = (view * eyePos).xyz;
	//vec3 viewDir = normalize(eyePosition - viewPos);
	vec3 viewDir = normalize(eyePos - fragPos);
	
	vec3 reflectDir = reflect(dir, norm);
	float sp = pow(max(dot(viewDir, reflectDir), 0.0), 16.0f);
	vec3 finalSpec = options.x * color.xyz * sp * specTex;
	
	// attenuation
	//float radius = pos.w;
	//float denom = dist / radius + 1.0f;
	//float att = 1.0f / (denom * denom);
	
	float att = 0.0f;
	
	// simple
	if (attCalc == 0)
	{
		att = 1 / pow(dist, 2.0f) - 1 / pow(pos.w, 2.0f);
	}
	
	// complex
	else
	{
		float r = 1.0f;
		float d = max(dist - r, 0.0f);
		
		//float att = (1.0f / pow(dist, 2)) - (1.0f / pow(pos.w, 2));
		float denom = (d / r) + 1.0f;
		att = 1.0f / pow(denom, 2.0f);
		
		att = (att - options.y) / (1.0f - options.y);
		att = max(att, 0.0f);
	}
	
	return att * (finalDiff + finalSpec);
}

void main()
{
	vec3 gFragPos = texture(gPos, gl_FragCoord.xy / vec2(1600, 900)).rgb;
	vec3 L = pos.xyz - gFragPos;
	float dist = length(L);
	
	if (dist > options.z)
	{
		discard;
	}

	fragColor = vec4(LightCalc(), 1.0f);
}
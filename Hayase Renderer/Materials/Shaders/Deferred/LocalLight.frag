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

layout(std140, binding = 2) uniform LocalLight
{
  vec4 pos; // xyz = position, w = range
  vec4 color;
  vec4 options; // x = intensity, y = cutoff, z = max range
};

uniform vec3 eyePos;
uniform int attCalc;

uniform int vWidth;
uniform int vHeight;

uniform int editorOffsetX;
uniform int editorOffsetY;

vec3 LightCalc()
{
	vec2 fragUV = vec2((gl_FragCoord.x) / (editorOffsetX + vWidth), 
	gl_FragCoord.y / (editorOffsetY + vHeight));
	
	vec3 gFragPos = texture(gPos, fragUV).rgb;
	vec3 norm = texture(gNorm, fragUV).rgb;
	vec3 diffTex = texture(gAlbedo, fragUV).rgb;
	vec3 specTex = texture(gSpecular, fragUV).rgb;

	vec3 L = pos.xyz - gFragPos;
	float dist = distance(gFragPos, pos.xyz);
	L /= dist;

	// ambient
	//vec3 amb = (diff * 0.2f);
	
	// diffuse
	float nDotL = max(dot(norm, L), 0.0);
	vec3 finalDiff = nDotL * diffTex * color.xyz;
	
	if (attCalc == 1)
	{
		finalDiff *= options.x;
	}
	
	// specular
	// correct?
	//vec3 eyePosition = (view * eyePos).xyz;
	//vec3 viewDir = normalize(eyePosition - viewPos);
	vec3 viewDir = normalize(eyePos - fragPos);
	
	vec3 reflectDir = reflect(L, norm);
	float sp = pow(max(dot(viewDir, reflectDir), 0.0), 16.0f);
	vec3 finalSpec = color.xyz * sp * specTex;
	
	if (attCalc == 1)
	{
		finalSpec *= options.x;
	}
	
	// attenuation
	//float radius = pos.w;
	//float denom = dist / radius + 1.0f;
	//float att = 1.0f / (denom * denom);
	
	float att = 0.0f;
	
	// simple
	if (attCalc == 0)
	{
		att = (1.0f / (dist * dist)) - (1.0f / (pos.w * pos.w));
	}
	
	// advanced
	else
	{
		float r = pos.w;
		//float d = max(dist - r, 0.0f);
		
		float test = (dist / r);
		
		//float att = (1.0f / pow(dist, 2)) - (1.0f / pow(pos.w, 2));
		float denom = (test) + 1.0f;
		att = 1.0f / pow(denom, 2.0f);
		
		att = (att - options.y) / (1.0f - options.y);
		att = max(att, 0.0f);
		
		if (test >= pos.w)
		{
			att = 0.0f;
		}
	}
	
	return att * (finalDiff + finalSpec);
}

void main()
{
	vec2 fragUV = vec2((gl_FragCoord.x) / (editorOffsetX + vWidth), 
	gl_FragCoord.y / (editorOffsetY + vHeight));

	vec3 gFragPos = texture(gPos, fragUV).rgb;
	vec3 L = pos.xyz - gFragPos;
	float dist = length(L);
	
	if (dist > pos.w)
	{
		discard;
	}

	fragColor = vec4(LightCalc(), 1.0f);
}
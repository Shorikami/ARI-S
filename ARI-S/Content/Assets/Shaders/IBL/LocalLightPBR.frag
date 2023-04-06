#version 450 core

out vec4 fragColor;

in vec3 fragPos;
in vec3 fragNorm;

layout (binding = 0) uniform sampler2D gPos;
layout (binding = 1) uniform sampler2D gNorm;
layout (binding = 2) uniform sampler2D gAlbedo;
layout (binding = 3) uniform sampler2D gMetRough;

uniform vec3 pos;
uniform vec4 color;

uniform vec3 eyePos;

uniform float range;
uniform float intensity;

uniform int vWidth;
uniform int vHeight;

const float PI = 3.14159265359f;

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float a = roughness;
    float k = pow(a, 2) / 2.0f;

    float nom   = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = pow(roughness, 2);
    float a2 = pow(a, 2);
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return nom / denom;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 LightCalc()
{
	vec2 fragUV = vec2(gl_FragCoord.x / vWidth, gl_FragCoord.y / vHeight);
	
	vec3 gFragPos = texture(gPos, fragUV).rgb;
	vec3 N = texture(gNorm, fragUV).rgb;
	vec3 albedo = texture(gAlbedo, fragUV).rgb;
	float rough = texture(gMetRough, fragUV).r;
    float metal = texture(gMetRough, fragUV).g;
	
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metal);
	
	vec3 L = normalize(pos - gFragPos);
    vec3 V = normalize(eyePos - gFragPos);
	
	vec3 H = normalize(V + L);
	float dist = length(pos - gFragPos);
	float att = ((1.0f / (dist * dist)) - (1.0f / pow(range, 2)));
	vec3 radiance = color.xyz * att;
	
	float NDF = DistributionGGX(N, H, rough);
	float G = GeometrySmith(N, V, L, rough);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);
	
	vec3 num = NDF * G * F;
	float denom = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
	vec3 finalSpec = num / denom;
	
	vec3 kS = F;
	vec3 kD = vec3(1.0f) - kS;
	kD *= 1.0f - metal;
	
	float nDotL = max(dot(N, L), 0.0f);
	
	return (kD * albedo / PI + finalSpec) * radiance * nDotL;

	//vec3 L = pos - gFragPos;
	//float dist = distance(gFragPos, pos);
	//L /= dist;
	//
	//// ambient
	////vec3 amb = (diff * 0.2f);
	//
	//// diffuse
	//float nDotL = max(dot(norm, L), 0.0);
	//vec3 finalDiff = intensity * nDotL * diffTex * color.xyz;
	//
	//// specular
	//vec3 viewDir = normalize(eyePos - fragPos);
	//
	//vec3 reflectDir = reflect(L, norm);
	//float sp = pow(max(dot(viewDir, reflectDir), 0.0), 16.0f);
	//vec3 finalSpec = intensity * color.xyz * sp * specTex;
	//
	//// attenuation
	//float att = ((1.0f / (dist * dist)) - (1.0f / pow(range, 2)));
	//
	//return att * (finalDiff + finalSpec);
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
	//if (dist > range)
	//{
	//	discard;
	//}

	fragColor = vec4(LightCalc(), 1.0f);
}
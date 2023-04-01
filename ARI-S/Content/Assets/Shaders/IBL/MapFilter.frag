#version 450 core

out vec4 fragColor;

in vec3 worldPos;

uniform samplerCube envMap;
uniform float roughness;

const float PI = 3.14159265359f;

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    // note that we use a different k for IBL
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

void main()
{
	vec3 N = normalize(worldPos);
	vec3 V = N;
	
	const uint sampleCount = 1024u;
	vec3 color = vec3(0.0f);
	float weight = 0.0f;
	
	for (uint i = 0u; i < sampleCount; ++i)
	{
		vec2 xi = Hammersley(i, sampleCount);
		vec3 H = ImportanceSampleGGX(xi, N, roughness);
		vec3 L = normalize(2.0f * dot(V, H) * H - V);
		
		float nDotL = max(dot(N, L), 0.0f);
		if (nDotL > 0.0f)
		{
			float D = DistributionGGX(N, H, roughness);
			float nDotH = max(dot(N, H), 0.0f);
			float hDotV = max(dot(H, V), 0.0f);
			float pdf = D * nDotH / (4.0f * hDotV) + 0.0001f;
			
			float resolution = 512.0f;
			float texel = 4.0f * PI / (6.0f * pow(resolution, 2));
			float txSample = 1.0f / (float(sampleCount) * pdf + 0.0001f);
			
			float mip = roughness == 0.0f ? 0.0f : 0.5f * log2(txSample / texel);
			
			color += textureLod(envMap, L, mip).rgb * nDotL;
			weight += nDotL;
		}
	}
	
	color = color / weight;
	
	fragColor = vec4(color, 1.0f);
}
out vec4 fragColor;

in vec3 worldPos;

uniform samplerCube envMap;
uniform float roughness;

layout(std140, binding = 4) uniform Discrepancy
{
  vec4 hammersley[100];
  int N;
} hammersleyVals;

void main()
{
	vec3 N = normalize(worldPos);
	vec3 V = N;
	
	const uint sampleCount = 20u;
	vec3 color = vec3(0.0f);
	float weight = 0.0f;
	
	for (uint i = 0u; i < sampleCount; ++i)
	{
		//vec2 xi = hammersleyVals.hammersley[i];
		//vec2 uv = (xi.x, atan((roughness * sqrt(xi.y)) / sqrt(1.0f - xi.y)));
		
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
			//float texel = 4.0f * PI / (6.0f * pow(resolution, 2));
			//float txSample = 1.0f / (float(sampleCount) * pdf + 0.0001f);
			
			float mip = roughness == 0.0f ? 0.0f : 0.5f * log2(pow(resolution, 2) / sampleCount) - 0.5f * D / 4;
			
			color += textureLod(envMap, L, mip).rgb * nDotL;
			weight += nDotL;
		}
	}
	
	color = color / weight;
	
	fragColor = vec4(color, 1.0f);
}
out vec2 fragColor;
in vec2 texCoords;

vec2 IntegrateBRDF(float nDotV, float roughness)
{
	vec3 V;
	V.x = sqrt(1.0f - pow(nDotV, 2));
	V.y = 0.0f;
	V.z = nDotV;
	
	float A = 0.0f;
	float B = 0.0f;
	
	vec3 N = vec3(0.0f, 0.0f, 1.0f);
	
	const uint sampleCount = 20u;
	
	for (uint i = 0u; i < sampleCount; ++i)
	{
		vec2 xi = Hammersley(i, sampleCount);
		vec3 H = ImportanceSampleGGX(xi, N, roughness);
		vec3 L = normalize(2.0f * dot(V, H) * H - V);
		
		float nDotL = max(L.z, 0.0f);
		float nDotH = max(H.z, 0.0f);
		float vDotH = max(dot(V, H), 0.0f);
		
		if (nDotL > 0.0f)
		{
			float G = GeometrySmith(N, V, L, roughness);
			float G_Vis = (G * vDotH) / (nDotH * nDotV);
			float Fc = pow(1.0f - vDotH, 5.0f);
			
			A += (1.0f - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	
	A /= float(sampleCount);
	B /= float(sampleCount);
	
	return vec2(A, B);
}

void main()
{
	fragColor = IntegrateBRDF(texCoords.x, texCoords.y);
}
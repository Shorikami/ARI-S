#version 450 core

out vec4 fragColor;
in vec2 texCoords;

layout (binding = 0) uniform sampler2D gPos;
layout (binding = 1) uniform sampler2D gNorm;
layout (binding = 2) uniform sampler2D gDepth;

uniform float aoScale;
uniform float aoContrast;
uniform int aoSamplePoints;
uniform float aoInfluenceRange;

uniform int vWidth;
uniform int vHeight;

const float PI = 3.14159265359f;

int Heaviside(float val)
{
	return val <= 0 ? 0 : 1;
}

float CalculateAO(vec2 coords, vec2 floatCoords, vec3 pos, vec3 norm, float depth)
{
	float sum = 0.0f;
	float c = 0.1f * aoInfluenceRange;
	float delta = 0.001f;
	
	for (int i = 0; i < aoSamplePoints; ++i)
	{
		float a = (i + 0.5f) / aoSamplePoints;
		float h = (a * aoInfluenceRange) / depth;
		
		float phi = (30 * int(coords.x) ^ int(coords.y)) + (10.0f * coords.x * coords.y);
		float theta = 2.0f * PI * a * ((7.0f * aoSamplePoints) / 9.0f) + phi;
		
		vec2 read = vec2(cos(theta), sin(theta)) * h;
		vec2 readFrom = floatCoords + read;
		
		vec3 pI = texture(gPos, readFrom).rgb;
		
		// ----
		
		vec3 wi = pI - pos;
		
		float step = Heaviside(aoInfluenceRange - length(wi));
		float numerator = max(0.0f, (dot(norm, wi) - delta * depth));
		float denominator = max(pow(c, 2), dot(wi, wi));
		
		sum += (numerator / denominator);
	}
	
	sum *= (2 * PI * c) / aoSamplePoints;
	
	return sum;
}



void main()
{	
	vec2 coords = vec2(gl_FragCoord.x, gl_FragCoord.y);
					
	vec2 uv = vec2(gl_FragCoord.x / vWidth, 
					gl_FragCoord.y / vHeight);
	
	vec3 fragPos = texture(gPos, uv).rgb;
	vec3 norm = texture(gNorm, uv).rgb;
	float depth = texture(gDepth, uv).r;
	
	float occlusion = CalculateAO(coords, uv, fragPos, norm, depth);
	float finalAO = max(0.0f, pow(1 - (aoScale * occlusion), aoContrast));

	fragColor = vec4(vec3(finalAO), 1.0f);
}
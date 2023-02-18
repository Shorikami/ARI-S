#version 450 core

layout(location = 0) out vec4 sDepth;

in vec4 shadowPos;

vec4 Moments(float fragDepth)
{
	float sq = fragDepth * fragDepth;
	return vec4(fragDepth, sq, sq * fragDepth, sq * sq);
}

void main()
{
	//float depth = shadowPos.z / shadowPos.w;
	//depth = depth * 0.5f + 0.5f;
	float depth = shadowPos.z;

	float z0 = -1.0f;
	float z1 = 1.0f;
	
	float final = (depth - z0) / (z1 - z0);
	
	sDepth = Moments(final);
}
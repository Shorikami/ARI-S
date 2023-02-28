#version 450 core

layout(location = 0) out vec4 sDepth;

in vec4 shadowPos;

uniform float nearP;
uniform float farP;
uniform bool usePersp;

vec4 Moments(float fragDepth)
{
	float sq = fragDepth * fragDepth;
	return vec4(fragDepth, sq, sq * fragDepth, sq * sq);
}

float Linearize(float depth)
{
	float z = depth * 2.0f - 1.0f;
	return (2.0f * nearP * farP) / (farP + nearP - z * (farP - nearP));
}

void main()
{
	float depth = shadowPos.z / shadowPos.w;
	//depth = depth * 0.5f + 0.5f;
	
	float relative = 0.0f;
	
	if (usePersp)
	{
		float linear = Linearize(depth) / farP;
		relative = (linear - -1) / (1 - -1);
	}
	else
	{
		relative = (depth - -1) / (1 - -1);
	}
	sDepth = Moments(relative);
}
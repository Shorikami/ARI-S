#version 450 core

layout(location = 0) out vec4 sDepth;

in vec4 shadowPos;

uniform float nearP;
uniform float farP;

vec4 Moments(float fragDepth)
{
	float sq = fragDepth * fragDepth;
	return vec4(fragDepth, sq, sq * fragDepth, sq * sq);
}

void main()
{
	float depth = shadowPos.z / shadowPos.w;
	//depth = depth * 0.5f + 0.5f;
	
	float relative = (depth - -1) / (1 - -1);
	
	sDepth = Moments(relative);
}
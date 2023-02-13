#version 430 core

layout(location = 0) out vec4 sDepth;

in vec4 shadowPos;

void main()
{
	// convert to [-1, 1]
	vec4 converted = shadowPos / shadowPos.w;
	
	// convert to [0, 1]
	vec4 newShadow = converted * 0.5f + vec4(0.5f);
	
	// (z, z^2, z^3, z^4; sampling from red component)
	//sDepth = shadowPos;
	sDepth = vec4(vec3(newShadow.r, pow(newShadow.r, 2), pow(newShadow.r, 3)), pow(newShadow.r, 4));
}
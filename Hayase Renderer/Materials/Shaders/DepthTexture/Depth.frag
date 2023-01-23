#version 430 core
layout (location = 0) out float gDepth;

in vec3 outPos;
in vec3 outNorm;
in vec2 outTexCoord;

void main()
{
	//gDepth = gl_FragCoord.z;
}
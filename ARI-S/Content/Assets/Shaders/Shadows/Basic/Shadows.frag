#version 430 core

layout(location = 0) out float sDepth;

void main()
{
	sDepth = gl_FragCoord.z;
}
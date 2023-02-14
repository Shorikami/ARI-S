#version 430 core

layout(location = 0) out vec4 sDepth;

void main()
{
	sDepth = vec4(vec3(gl_FragCoord.z, pow(gl_FragCoord.z, 2), pow(gl_FragCoord.z, 3)), pow(gl_FragCoord.z, 4));
}
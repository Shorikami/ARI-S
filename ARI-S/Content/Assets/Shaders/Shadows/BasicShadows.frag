#version 430 core

layout(location = 0) out vec4 sDepth;

in vec4 shadowPos;

void main()
{
	sDepth = shadowPos;
}
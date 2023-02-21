#version 450 core

layout (location = 0) in vec3 aPosition;

uniform mat4 proj;
uniform mat4 view;

void main()
{
	vec4 resVec = vec4(aPosition, 1.0f);
	gl_Position = resVec / resVec.w;
}
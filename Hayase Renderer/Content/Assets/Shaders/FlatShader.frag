#version 430 core

layout(std140, binding = 2) uniform LocalLight
{
  vec4 pos; // xyz = position, w = range
  vec4 color;
};

in vec3 fragPos;
in vec3 fragNorm;

out vec4 fragColor;

void main()
{
	fragColor = vec4(vec3(color), 1.0);
}

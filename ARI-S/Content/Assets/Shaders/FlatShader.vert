#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;

uniform mat4 model;

layout(std140, binding = 0) uniform World
{
  mat4 proj;
  mat4 view;
  vec2 nearFar;
};

out vec3 fragPos;
out vec3 fragNorm;

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0f);
	
	gl_Position = proj * view * worldPos;
	
	fragPos = worldPos.xyz;
	fragNorm = mat3(transpose(inverse(mat3(model)))) * aNormals;
}

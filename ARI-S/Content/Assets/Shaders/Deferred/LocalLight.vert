#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;

out vec3 fragPos;
out vec3 fragNorm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0f);
	fragPos = worldPos.xyz;
	
	mat3 normalMat = transpose(inverse(mat3(model)));
	fragNorm = normalMat * aNormals;

	gl_Position = projection * view * worldPos;
}
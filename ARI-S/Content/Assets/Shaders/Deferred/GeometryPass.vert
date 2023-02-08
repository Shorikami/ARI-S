#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec2 aTexCoords;

out vec3 outPos;
out vec3 outNorm;
out vec2 outTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0f);
	outPos = worldPos.xyz;
	
	mat3 normalMat = transpose(inverse(mat3(model)));
	outNorm = normalMat * aNormals;
	
	outTexCoord = aTexCoords;
	
	gl_Position = projection * view * worldPos;
}
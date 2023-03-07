#version 450 core
layout (location = 0) in vec3 aPosition;

out vec3 worldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	worldPos = aPosition;
	mat4 newView = mat4(mat3(view));
	vec4 clipPosition = projection * newView * vec4(worldPos, 1.0f);
	
	gl_Position = clipPosition.xyww;
}
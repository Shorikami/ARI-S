#version 430 core

out vec4 fragColor;

uniform sampler2D depthMap;

in vec2 texCoords;

void main()
{
	float depth = texture(depthMap, texCoords).r;
	fragColor = vec4(vec3(depth), 1.0f);
}
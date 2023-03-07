#version 450 core

out vec4 fragColor;

in vec3 worldPos;

uniform samplerCube environmentMap;

void main()
{
	vec3 color = texture(environmentMap, worldPos).rgb;
	float e = 1.0f;
	
	color = (e * color) / (e * color + vec3(1.0f));
	color = pow(color, vec3(1.0f / 2.2f));
	
	fragColor = vec4(color, 1.0f);
}
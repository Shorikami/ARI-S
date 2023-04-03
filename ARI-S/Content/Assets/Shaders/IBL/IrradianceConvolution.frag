#version 450 core

out vec4 fragColor;

in vec3 worldPos;

uniform samplerCube envMap;

const float PI = 3.14159265359f;

void main()
{
	vec3 N = normalize(worldPos);
	
	vec3 irradiance = vec3(0.0f);
	
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 right = normalize(cross(up, N));
	up = normalize(cross(N, right));
	
	float delta = 0.025f;
	float sampleCount = 0.0f;
	
	for (float phi = 0.0f; phi < 2.0f * PI; phi += delta)
	{
		for (float theta = 0.0f; theta < 0.5f * PI; theta += delta)
		{
			vec3 tangent = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			
			vec3 sampleVec = tangent.x * right + tangent.y * up + tangent.z * N;
			
			irradiance += texture(envMap, sampleVec).rgb * cos(theta) * sin(theta);
			++sampleCount;
		}
	}
	
	irradiance = PI * irradiance * (1.0f / float(sampleCount));
	fragColor = vec4(irradiance, 1.0f);
}
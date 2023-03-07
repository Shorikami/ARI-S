#version 450 core
#define MATH_PI 3.1415926535897932384626433832795

out vec4 fragColor;

in vec3 worldPos;

uniform sampler2D hdrMap;

vec2 DirToUV(vec3 dir)
{
	//return vec2(0.5f - (atan(dir.y, dir.x) / (2.0f * MATH_PI)), (acos(dir.z) / MATH_PI));
	return vec2(0.5f - (atan(dir.z, dir.x) / (2.0f * MATH_PI)), (1.0f - acos(dir.y) / MATH_PI));
	//return vec2(0.5f + 0.5f * atan(dir.z, dir.x) / MATH_PI, 1.0f - acos(dir.y) / MATH_PI);
}

void main()
{
	vec2 uv = DirToUV(normalize(worldPos));
	vec3 res = texture(hdrMap, uv).rgb;
	
	fragColor = vec4(res, 1.0f);
}
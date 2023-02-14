#version 430 core

out vec4 fragColor;

uniform sampler2D uShadowMap;

uniform int vWidth;
uniform int vHeight;

void main()
{
	vec2 uv = vec2(gl_FragCoord.x / vWidth, 
					gl_FragCoord.y / vHeight);

    float depthValue = texture(uShadowMap, uv).r;
    fragColor = vec4(vec3(depthValue), 1.0);
}
#version 430 core

in VS_OUT
{
    vec3 fragDiffuse;
    vec3 fragNormal;

} fs_in;


out vec4 fragColor;

uniform sampler2D uShadowMap;
in vec4 shadowCoord;

uniform int vWidth;
uniform int vHeight;

float Shadow(vec4 v, float bias)
{
	// shadowCoord is already converted to [0, 1]
	//vec3 coords = v.xyz / v.w;
	//coords = coords * 0.5f + 0.5f;
	
	float closest = texture(uShadowMap, v.xy).r;
	float curr = v.z;
	
	return curr - bias > closest ? 1.0f : 0.0f;
}

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(vWidth, vHeight);

	vec3 lightColor = vec3(0.5f);
	vec3 lightPos = vec3(1.0f);

    // Ambient
	float ambientStrength = 0.9f;
	vec4 ambient = vec4(ambientStrength * lightColor, 1.0f);
	
	// Diffuse
	vec3 lNormal = normalize(lightPos);
	float dotP = dot(fs_in.fragNormal, lNormal);
	float nDotL = max(dotP, 0.0f);
	vec4 diffuse = vec4(lightColor * nDotL, 1.0f);
	
	float bias = max(0.025f * (1.0f - dot(lNormal, lightPos)), 0.0025f);
	
	float shadow = Shadow(shadowCoord, bias);
	fragColor = ambient + (1.0f - shadow) * diffuse;
}

#version 430 core

in VS_OUT
{
    vec3 fragDiffuse;
    vec3 fragNormal;

} fs_in;


out vec4 fragColor;

void main()
{
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
	
	
	fragColor = vec4(fs_in.fragDiffuse, 1.0) * (ambient + diffuse);
}

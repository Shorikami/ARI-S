#version 430 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 modelPosition;
layout (location = 1) in vec3 vertexNormal;

vec3 diffuseColor = vec3( 0.4f, 0.4f, 0.4f );

out VS_OUT
{
    vec3 fragDiffuse;
    vec3 fragNormal;

} vs_out;

void main()
{
    vs_out.fragDiffuse = diffuseColor;
	
	mat3 normalTransf = mat3(transpose(inverse(model)));
    vs_out.fragNormal = normalize(normalTransf * vertexNormal);

	gl_Position = projection * view * model * vec4(modelPosition, 1.0f);
}

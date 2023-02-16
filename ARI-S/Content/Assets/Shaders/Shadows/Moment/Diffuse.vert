#version 430 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

vec3 diffuseColor = vec3( 0.4f, 0.4f, 0.4f );

uniform mat4 shadowMatrix;

out VS_OUT
{
    vec3 fragDiffuse;
    vec3 fragNormal;
    vec2 texCoords;
    
    vec4 shadowCoord;

} vs_out;

void main()
{
    vs_out.fragDiffuse = diffuseColor;
	
	  mat3 normalTransf = mat3(transpose(inverse(model)));
    vs_out.fragNormal = normalize(normalTransf * aNorm);

	  gl_Position = projection * view * model * vec4(aPos, 1.0f);

	  vs_out.shadowCoord = shadowMatrix * model * vec4(aPos, 1.0f);
}

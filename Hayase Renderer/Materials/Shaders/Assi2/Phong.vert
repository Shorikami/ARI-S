#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;

layout(std140, binding = 0) uniform World
{
  mat4 proj;
  mat4 view;
  vec2 nearFar;
};

out VS_OUT
{
  vec3 viewPos;
  vec3 aPosition;
  vec3 aNorm;
  vec2 aTex;
} vs_out;

void main()
{
  mat4 modelView = view * model;
  mat3 normMatrix = mat3(transpose(inverse(model)));

  vs_out.viewPos = (model * vec4(aPos, 1.0)).xyz;
  vs_out.aPosition = aPos;
  vs_out.aNorm = normMatrix * aNormal;
  vs_out.aTex = aTexCoords;
  
  gl_Position = proj * modelView * vec4(aPos, 1.0);
}
#version 430 core

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
  vec3 aPosition;
} vs_out;

void main()
{
  mat4 modelView = view * model;

  vs_out.aPosition = aPos;
  
  gl_Position = (proj * view * vec4(aPos, 1.0)).xyww;
}
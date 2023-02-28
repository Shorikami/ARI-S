#version 450 core

uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 modelPosition;

void main()
{
  gl_Position = projection * view * vec4(modelPosition, 1.0);
}
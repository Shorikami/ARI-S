#version 410 core

in VS_OUT
{
  vec4 finalColor;
} fs_in;

out vec4 fragColor;

void main()
{
  fragColor = fs_in.finalColor;
}
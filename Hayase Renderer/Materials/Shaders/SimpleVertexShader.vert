#version 410 core

uniform mat4  vertexTransform;

out gl_PerVertex
{
    vec4      gl_Position;
    float     gl_PointSize;
};

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3    vPosition;
layout(location = 1) in vec3 vertexNormal;
//layout(location = 1) in float   vDiffuse;
//layout(location = 2) in vec3    vColor;
//layout(location = 3) in vec2    vUV;


// Interpolating vertex attributes over the rasterizer
out VS_OUT
{
    vec3 rasterColor;
    vec4 vertexPosition;
    vec3 vertexNormal;
//    float diffuseFactor;
//    vec2 vertexUV;
    
} vs_out;

void main()
{
    vs_out.rasterColor = vertexNormal;
    vs_out.vertexPosition = vertexTransform *  vec4( vPosition, 1.0f );
    vs_out.vertexNormal = vertexNormal;
//    vs_out.diffuseFactor = vDiffuse;
//    vs_out.vertexUV = vUV;
    
    gl_Position = vs_out.vertexPosition;
}

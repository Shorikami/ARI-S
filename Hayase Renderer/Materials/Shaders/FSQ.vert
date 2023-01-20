
uniform mat4  vertexTransform;

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3    vPosition;
layout(location = 1) in vec3    vColor;

// Interpolating vertex attributes over the rasterizer
out VS_OUT
{
    vec4 vertexPosition; // interpolated vPosition
    vec3 vertexColor;

} vs_out;

out mat4  vTransform;

void main()
{
    vs_out.vertexPosition = vec4( vPosition, 1.0f );
    vs_out.vertexColor = vColor;

    vTransform = vertexTransform;

    gl_Position = vs_out.vertexPosition;
}

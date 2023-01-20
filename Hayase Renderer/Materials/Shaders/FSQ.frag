

uniform sampler2D texSampler;

// Interpolating vertex attributes over the rasterizer
in GS_OUT
{
    vec2 vertexUV;
    vec3 vertexColor;

} fs_in;

out vec3 fragColor;

void main()
{
    vec3 tColor = texture( texSampler, fs_in.vertexUV ).rgb;
    vec3 fColor = fs_in.vertexColor;
    fragColor = mix( tColor, fColor, 0.015f );
}

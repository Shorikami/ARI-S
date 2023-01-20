#version 410 core

// Input from the rasterizer
// Conceptually equivalent to
// "WritePixel( x, y, rasterColor )"
in VS_OUT
{
    vec3 rasterColor;
    vec4 vertexPosition;
    vec3 vertexNormal;
//    float diffuseFactor;
//    vec2 vertexUV;

} fs_in;

//vec3 normal = normalize(fs_in.vertexNormal);

// Output data
out vec3 color;


void main()
{

	// Output color = bilinearly interpolated rasterColor
//	color = vec3(fs_in.diffuseFactor);
	color = fs_in.rasterColor;
}

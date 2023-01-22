#version 410 core

out vec4 fragColor;

uniform sampler2D cubeTexture[6];

in VS_OUT
{
  vec3 aPosition;
} fs_in;

vec2 CubemapUV(vec3 v, out int idx)
{
	float x = v.x;
	float y = v.y;
	float z = v.z;
	
	float absX = abs(x);
	float absY = abs(y);
	float absZ = abs(z);

	int isXPositive = x > 0 ? 1 : 0;
	int isYPositive = y > 0 ? 1 : 0;
	int isZPositive = z > 0 ? 1 : 0;
	
	float maxAxis, uc, vc;
	vec2 uv = vec2(0.0);
	
	// POSITIVE X
	if (bool(isXPositive) && (absX >= absY) && (absX >= absZ))
	{
		// u (0 to 1) goes from +z to -z
		// v (0 to 1) goes from -y to +y
		maxAxis = absX;
		uc = -z;
		vc = y;
		idx = 0;
	}
	
		// NEGATIVE X
	else if (!bool(isXPositive) && absX >= absY && absX >= absZ)
	{
		// u (0 to 1) goes from -z to +z
		// v (0 to 1) goes from -y to +y
		maxAxis = absX;
		uc = z;
		vc = y;
		idx = 1;
	}
	
		// POSITIVE Y
	else if (bool(isYPositive) && absY >= absX && absY >= absZ)
	{
		// u (0 to 1) goes from -x to +x
		// v (0 to 1) goes from +z to -z
		maxAxis = absY;
		uc = x;
		vc = -z;
		idx = 2;
	}
	
		// NEGATIVE Y
	else if (!bool(isYPositive) && absY >= absX && absY >= absZ)
	{
		// u (0 to 1) goes from -x to +x
		// v (0 to 1) goes from -z to +z
		maxAxis = absY;
		uc = x;
		vc = z;
		idx = 3;
	}
	
		// POSITIVE Z
	else if (bool(isZPositive) && absZ >= absX && absZ >= absY)
	{
		// u (0 to 1) goes from -x to +x
		// v (0 to 1) goes from -y to +y
		maxAxis = absZ;
		uc = x;
		vc = y;
		idx = 4;
	}
	
		// NEGATIVE Z
	else if (!bool(isZPositive) && absZ >= absX && absZ >= absY)
	{
		// u (0 to 1) goes from +x to -x
		// v (0 to 1) goes from -y to +y
		maxAxis = absZ;
		uc = -x;
		vc = y;
		idx = 5;
	}
	
	// Convert range from -1 to 1 to 0 to 1
	uv.s = 0.5f * (uc / maxAxis + 1.0f);
	uv.t = 0.5f * (vc / maxAxis + 1.0f);
	
	return uv;
}

vec3 SampleCubemap(vec3 v)
{
    int index = 0;
    vec2 uv = CubemapUV(v, index);
    return texture(cubeTexture[index], uv).rgb;
}

void main()
{
    vec3 color = SampleCubemap(fs_in.aPosition);
	fragColor = vec4(color, 1.0);
}
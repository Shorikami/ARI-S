#version 410 core

out vec4 fragColor;

//uniform sampler2D diffTex;
//uniform sampler2D specTex;
uniform sampler2D cubeTex[6];

uniform int refType;

uniform float refractIdx;
uniform float fresPower;
uniform float fresConstant;

in VS_OUT
{
  vec3 viewPos;   // for light
  vec3 aPosition; // for texCoords
  vec3 aNorm;
  vec2 aTex;
} fs_in;

layout(std140, binding = 0) uniform World
{
  mat4 proj;
  mat4 view;
  vec2 nearFar;
};

layout(std140, binding = 1) uniform Lights
{
  vec4 lightPos[16];
  vec4 lightColor[16];
  vec4 lightDir[16];
  
  vec4 eyePos;
  vec4 emissive;
  vec4 globalAmbient;
  vec4 coefficients;

  vec4 fogColor;
  
  vec4 specular[16];
  vec4 ambient[16];
  vec4 diffuse[16];
  
  // x = inner, y = outer, z = falloff, w = type
  vec4 lightInfo[16];
  
  ivec4 modes;
 
  vec3 attenuation;
  int numLights;
};

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

vec2 CreateUV(vec3 v, float min, float max, int mode)
{
	float PI = 3.14159f;
	
	// planar
	if (mode == 0)
	{
		float u = (v.x + min) / (max - min);
		float v = (v.y + min) / (max - min);
		return vec2(u, v);
	}
	
	// cylindrical
	else if (mode == 1)
	{
		float theta = degrees(atan(v.y, v.x));
		float z = (v.z + 1.0f) * 0.5f;
		
		float u = theta / 360.0f;
		float v = z;
		return vec2(u, v);
	}
	
	// spherical
	else if (mode == 2)
	{
		float theta = atan(v.y, v.x);
		float r = v.x * v.x + v.y * v.y + v.z * v.z;
		r = sqrt(r);
		float pheta = acos(v.z / r);
		float u = theta / (2 * PI);
		float v = (PI - pheta) / PI;
		return vec2(u, v);
	}
	
	// cube map
	else
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
		}
	
			// NEGATIVE X
		else if (!bool(isXPositive) && absX >= absY && absX >= absZ)
		{
			// u (0 to 1) goes from -z to +z
			// v (0 to 1) goes from -y to +y
			maxAxis = absX;
			uc = z;
			vc = y;
		}
	
			// POSITIVE Y
		else if (bool(isYPositive) && absY >= absX && absY >= absZ)
		{
			// u (0 to 1) goes from -x to +x
			// v (0 to 1) goes from +z to -z
			maxAxis = absY;
			uc = x;
			vc = -z;
		}
	
			// NEGATIVE Y
		else if (!bool(isYPositive) && absY >= absX && absY >= absZ)
		{
			// u (0 to 1) goes from -x to +x
			// v (0 to 1) goes from -z to +z
			maxAxis = absY;
			uc = x;
			vc = z;
		}
	
			// POSITIVE Z
		else if (bool(isZPositive) && absZ >= absX && absZ >= absY)
		{
			// u (0 to 1) goes from -x to +x
			// v (0 to 1) goes from -y to +y
			maxAxis = absZ;
			uc = x;
			vc = y;
		}
	
			// NEGATIVE Z
		else if (!bool(isZPositive) && absZ >= absX && absZ >= absY)
		{
			// u (0 to 1) goes from +x to -x
			// v (0 to 1) goes from -y to +y
			maxAxis = absZ;
			uc = -x;
			vc = y;
		}
	
		// Convert range from -1 to 1 to 0 to 1
		uv.s = 0.5f * (uc / maxAxis + 1.0f);
		uv.t = 0.5f * (vc / maxAxis + 1.0f);
		
		return uv;
	}
}

vec3 CubemapSampler(vec3 v)
{
	int idx = 0;
	vec2 uv = CubemapUV(normalize(v), idx);
	return texture(cubeTex[idx], uv).rgb;
}

vec3 reflectVec(vec3 L, vec3 N)
{
	return L - 2 * dot(N, L) * N;
}

vec3 refraction(vec3 i, vec3 n, float ratio)
{	
	float k = 1.0f - ratio * ratio * (1.0f - dot(n, i) * dot(n, i));
	
	if (k < 0.0f)
    {
		return vec3(0.0f);
	}
	else
	{
		return ratio * i - (ratio * dot(n, i) + sqrt(k)) * n;
	}
}

vec3 reflectEnv(vec3 eye)
{
	vec3 i = normalize(fs_in.viewPos - eye);
	vec3 n = normalize(fs_in.aNorm);
	vec3 r = normalize(reflectVec(i, n));
	
	return CubemapSampler(r);
}

vec3 refractEnv(vec3 eye, float ratio)
{
	vec3 i = normalize(fs_in.viewPos - eye);
	vec3 n = normalize(fs_in.aNorm);
	vec3 r = vec3(1.0f);
	
	vec3 red = refraction(i, n, ratio - 0.01f);
	vec3 green = refraction(i, n, ratio);
	vec3 blue = refraction(i, n, ratio + 0.01f);
	
	//red.x = -red.x;
	//green.x = -green.x;
	//blue.x = -blue.x;
	//
	//red.y = -red.y;
	//green.y = -green.y;
	//blue.y = -blue.y;
	
	r.x = CubemapSampler(red).x;
	r.y = CubemapSampler(green).y;
	r.z = CubemapSampler(blue).z;
	
	return r;
}

float attValue(float c1, float c2, float c3, float dist)
{
	return min(1.0 / (c1 + c2 * dist + c3 * dist * dist), 1.0);
}

float fogValue(float n, float f, float v)
{
	return (f - v) / (f - n);
}

float Spotlight(int id, vec3 v)
{
	vec3 L = v;
	vec3 D = normalize(-lightDir[id].xyz);
	
	float phi = radians(lightInfo[id].x);
	float theta = radians(lightInfo[id].y);
	float falloff = lightInfo[id].z;
	
	if (dot(L, D) < cos(phi))
	{
		return 0.0f;
	}
	else if (dot(L, D) > cos(theta))
	{
		return 1.0f;
	}
	else
	{
		float alpha = acos(dot(L, D) / (length(L) * length(D)));
		return pow((cos(alpha) - cos(phi)) / (cos(theta) - cos(phi)), falloff);
	}
}

vec3 LightCalc(int id)
{
	int genUV = modes.x;
	int useNormals = modes.y;
	int uvType = modes.z;
	
	vec2 genTex;
	
	if (useNormals == 0)
	{
		genTex = CreateUV(fs_in.aPosition, -1.0, 1.0, uvType);
	}
	else
	{
		genTex = CreateUV(normalize(fs_in.aNorm), 0.0, 1.0, uvType);
	}
	
	vec2 finalTex = genUV == 0 ? fs_in.aTex : genTex;

	//vec3 textureDiff = texture(diffTex, finalTex).xyz;
	//vec3 textureSpec = texture(specTex, finalTex).xyz;
	
	// ambient
	vec3 amb = coefficients.x * ambient[id].xyz;
	
	// diffuse
	vec3 norm = normalize(fs_in.aNorm);
	
	float mode = lightInfo[id].w;
	vec3 dir;
	if (mode == 0.0 || mode == 2.0)
	{
		dir = normalize(lightPos[id].xyz - fs_in.viewPos);
	}
	else
	{
		dir = -lightDir[id].xyz;
	}
	
	float nDotL = max(dot(norm, dir), 0.0);
	vec3 diff = coefficients.y * nDotL  * diffuse[id].xyz;
	
	// specular
	vec3 viewDir = normalize(eyePos.xyz - fs_in.viewPos);
	vec3 reflectDir = reflectVec(dir, norm);
	float sp = pow(max(dot(viewDir, reflectDir), 0.0), 255);
	vec3 spec = coefficients.z * sp * specular[id].xyz;
	
	// attenuation
	float dist = length(lightPos[id].xyz - fs_in.viewPos);
	float att = attValue(attenuation.x, attenuation.y, attenuation.z, dist);
	
	// spotlight
	float spotlightEffect = 1.0;
	if (mode == 2.0)
	{
		spotlightEffect = Spotlight(id, dir);
	}
	
	diff *= spotlightEffect;
	spec *= spotlightEffect;

	return att * amb + att * (diff + spec);
}


void main()
{
	vec3 em = emissive.xyz;
	vec3 localLight;
	
	for (int i = 0; i < numLights; ++i)
	{
		localLight += LightCalc(i);
	}
	
	// fog value
	vec3 eye = (view * eyePos).xyz;
	vec3 v = normalize(eye - fs_in.viewPos);
	float s = fogValue(nearFar.x, nearFar.y, length(v));
	
	vec3 local = em + globalAmbient.xyz * coefficients.x + localLight;
	vec3 final = s * local + (1 - s) * fogColor.xyz;
	
	// FRESNAL
	vec3 i = normalize(fs_in.viewPos - eye);
	vec3 n = normalize(fs_in.aNorm);
	float eta = 1.0f / refractIdx;
	float f = ((1.0f - eta) * (1.0f - eta)) / ((1.0f + eta) * (1.0f + eta));
	float ratio = f + (1.0f - f) * pow((1.0f - dot(i, n)), 1.0f);
	
	vec3 reflectedColor = reflectEnv(eyePos.xyz);
	vec3 refractedColor = refractEnv(eyePos.xyz, eta);

	if (refType == 0)
	{
		fragColor = vec4(reflectedColor, 1.0);
	}
	
	else if (refType == 1)
	{
		fragColor = vec4(refractedColor, 1.0);
	}
	
	else if (refType == 2)
	{
		vec3 rfr = refractEnv(eyePos.xyz, refractIdx);
		vec3 envColor = mix(reflectedColor, rfr, fresPower);
		fragColor = vec4(envColor, 1.0);
	}
	
	
	//fragColor = vec4(final, 1.0);
}
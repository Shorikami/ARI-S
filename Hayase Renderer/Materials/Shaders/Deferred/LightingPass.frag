#version 410 core

out vec4 fragColor;

layout (binding = 0) uniform sampler2D gPos;
layout (binding = 1) uniform sampler2D gNorm;
layout (binding = 2) uniform sampler2D gUVs;
layout (binding = 3) uniform sampler2D gAlbedo;
layout (binding = 4) uniform sampler2D gSpecular;
layout (binding = 5) uniform sampler2D gDepth;

uniform vec3 viewPos;

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

float attValue(float c1, float c2, float c3, float dist)
{
	return min(1.0 / (c1 + c2 * dist + c3 * dist * dist), 1.0);
}

vec3 LightCalc(int id)
{
	vec3 fragPos = texture(gPos, gl_FragCoord.xy / vec2(1600, 900)).rgb;
	vec3 norm = texture(gNorm, gl_FragCoord.xy / vec2(1600, 900)).rgb;
	vec2 uv = texture(gUVs, gl_FragCoord.xy / vec2(1600, 900)).rg;
	vec3 diff = texture(gAlbedo, gl_FragCoord.xy / vec2(1600, 900)).rgb;
	float spec = texture(gDepth, gl_FragCoord.xy / vec2(1600, 900)).r;

	// ambient
	vec3 amb = coefficients.x * (diff * 0.2f);
	
	// diffuse
	vec3 dir = normalize(lightPos[id].xyz - fragPos);
	float nDotL = max(dot(norm, dir), 0.0);
	vec3 finalDiff = coefficients.y * nDotL * diff;
	
	// specular
	// correct?
	vec3 eyePosition = (view * eyePos).xyz;
	vec3 viewDir = normalize(eyePosition - viewPos);
	//vec3 viewDir = normalize(eyePos.xyz - viewPos);
	
	vec3 reflectDir = reflect(dir, norm);
	float sp = pow(max(dot(viewDir, reflectDir), 0.0), 16.0f);
	vec3 finalSpec = lightColor[id].xyz * coefficients.z * sp * spec;
	
	// attenuation
	float dist = length(lightPos[id].xyz - fragPos);
	float att = attValue(attenuation.x, attenuation.y, attenuation.z, dist);
	
	return att * amb + att * (finalDiff + finalSpec);
}

void main()
{
	vec3 localLight = vec3(0.0f);
	
	for (int i = 0; i < numLights; ++i)
	{
		localLight += LightCalc(i);
	}
	
	fragColor = vec4(localLight, 1.0f);
}
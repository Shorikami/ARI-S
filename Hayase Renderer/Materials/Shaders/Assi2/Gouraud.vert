#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;

uniform sampler2D diffTex;
uniform sampler2D specTex;

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

out VS_OUT
{
  vec4 finalColor;
} vs_out;

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
		float theta = atan(v.y, v.x);
		float z = (v.z + min) / (max - min);
		
		float u = theta / (2 * PI);
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

vec3 reflectVec(vec3 L, vec3 N)
{
	return 2 * dot(N, L) * N - L;
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

vec3 LightCalc(int id, vec3 aNorm, vec3 viewPos)
{
	int genUV = modes.x;
	int useNormals = modes.y;
	int uvType = modes.z;
	
	vec2 genTex;
	
	if (useNormals == 0)
	{
		genTex = CreateUV(aPos, -1.0, 1.0, uvType);
	}
	else
	{
		genTex = CreateUV(normalize(aNorm), 0.0, 1.0, uvType);
	}
	
	vec2 finalTex = genUV == 0 ? aTexCoords : genTex;

	vec3 textureDiff = texture(diffTex, finalTex).xyz;
	vec3 textureSpec = texture(specTex, finalTex).xyz;
	
	// ambient
	vec3 amb = coefficients.x * ambient[id].xyz;
	
	// diffuse
	vec3 norm = normalize(aNorm);
	
	float mode = lightInfo[id].w;
	vec3 dir;
	if (mode == 0.0 || mode == 2.0)
	{
		dir = normalize(lightPos[id].xyz - viewPos);
	}
	else
	{
		dir = -lightDir[id].xyz;
	}
	
	float nDotL = max(dot(norm, dir), 0.0);
	vec3 diff = coefficients.y * nDotL * textureDiff * diffuse[id].xyz;
	
	// specular
	vec3 viewDir = normalize(eyePos.xyz - viewPos);
	vec3 reflectDir = reflectVec(dir, norm);
	float sp = pow(max(dot(viewDir, reflectDir), 0.0), textureSpec.r * textureSpec.r * 255);
	vec3 spec = coefficients.z * sp * specular[id].xyz * textureSpec;
	
	// attenuation
	float dist = length(lightPos[id].xyz - viewPos);
	float att = attValue(attenuation.x, attenuation.y, attenuation.z, dist);
	
	// spotlight
	float spotlightEffect = 1.0;
	if (mode == 2.0)
	{
		spotlightEffect = Spotlight(id, dir);
	}
	
	diff *= spotlightEffect;
	spec *= spotlightEffect;
	
	if (mode == 1.0)
	{
		return (amb + diff + spec);
	}
	
	return att * amb + att * (diff + spec);
}

void main()
{
  // Position
  mat4 modelView = view * model;
  mat3 normMatrix = mat3(transpose(inverse(model)));

  vec3 viewPos = (model * vec4(aPos, 1.0)).xyz;
  vec3 aNorm = normMatrix * aNormal;
  
  gl_Position = proj * modelView * vec4(aPos, 1.0);

  // Lighting
  vec3 em = emissive.xyz;
  vec3 localLight;
  
  for (int i = 0; i < numLights; ++i)
  {
  	localLight += LightCalc(i, aNorm, viewPos);
  }
  
  // fog value
  vec3 eye = (view * eyePos).xyz;
  vec3 v = normalize(eye - viewPos);
  float s = fogValue(nearFar.x, nearFar.y, length(v));
  
  vec3 local = em + globalAmbient.xyz * coefficients.x + localLight;
  vec3 final = s * local + (1 - s) * fogColor.xyz;
  
  vs_out.finalColor = vec4(final, 1.0);
}
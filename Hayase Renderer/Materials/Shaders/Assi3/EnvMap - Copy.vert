#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform float refractIdx;

uniform mat4 model;

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
  vec3 viewPos;
  vec3 aPosition;
  vec3 aNorm;
  vec2 aTex;
  vec3 test;
} vs_out;

vec3 refraction(vec3 i, vec3 n, float ratio)
{
	//vec3 l = -i;
	//float nl = dot(n, l);
	//float sinT2 = pow(ratio, 2.f) * (1.f - pow(nl, 2.f));
	//
	//if (sinT2 > 1.f)
	//{
	//	vec3 tirvec = i - 2 * n * dot(n, i);
	//	return tirvec;
	//}
	//
	//float cosT = sqrt(1.f - sinT2);
	//vec3 rfr_vec = ratio * l + (ratio * nl - cosT) * n;
	//rfr_vec.x *= -1.f;
	//
	//return rfr_vec;

	vec3 l = -i;
	float nl = dot(n, l);
	
	vec3 rfr_vec = (ratio * nl - sqrt(1.f - pow(ratio, 2.f) * (1.f - pow(nl, 2.f)))) * n - ratio * l;
	
	//rfr_vec.y *= -1.f;
	rfr_vec.z *= -1.f;
	
	return rfr_vec;
	
	//float k = 1.0f - ratio * ratio * (1.0f - dot(n, i) * dot(n, i));
	//
	//if (k < 0.0f)
    //{
	//	return vec3(0.0f);
	//}
	//else
	//{
	//	return ratio * i - (ratio * dot(n, i) + sqrt(k)) * n;
	//}
}

void main()
{
  mat4 modelView = view * model;
  mat4 normMatrix = inverse(model);
  
  vec3 i = vec3(model * vec4(aPos - vec3(eyePos), 1));
  
  vs_out.aPosition = aPos;
  vs_out.aNorm = mat3(normMatrix) * aNormal;
  vec4 a = normMatrix * vec4(aNormal, 1);
  vs_out.aTex = aTexCoords;

  float eta = 1.0f / refractIdx;

  vs_out.viewPos = (model * vec4(aPos, 1.0)).xyz;
  vs_out.test = refraction(i, vec3(a), eta);
  
  gl_Position = proj * modelView * vec4(aPos, 1.0);
}
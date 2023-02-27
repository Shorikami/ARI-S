#version 450 core

out vec4 fragColor;

layout (binding = 0) uniform sampler2D gPos;
layout (binding = 1) uniform sampler2D gNorm;
layout (binding = 2) uniform sampler2D gUVs;
layout (binding = 3) uniform sampler2D gAlbedo;
layout (binding = 4) uniform sampler2D gSpecular;
layout (binding = 5) uniform sampler2D gDepth;

uniform sampler2D uShadowMap;

uniform mat4 worldToLightMat;

uniform vec3 lightDir;
uniform vec3 viewPos;

uniform int vWidth;
uniform int vHeight;

vec2 Quadratic(float a, float b, float c)
{
	float discriminant = b * b - 4 * a * c;
	float x1 = (-b + sqrt(discriminant)) / (2 * a);
	float x2 = (-b - sqrt(discriminant)) / (2 * a);
  
	if (x1 > x2)
	{
		return vec2(x2, x1);
	}
  
	return vec2(x1, x2);
}

vec3 Cholesky(float m11, float m12, float m13, float m22, float m23, float m33, 
			  float z1, float z2, float z3)
{
	float a = sqrt(m11);
	if (a <= 0.0f)
	{
		a = 1.0f * pow(10.0f, -4);
	}
	
	float b = m12 / a;
	float c = m13 / a;
	
	float d = sqrt(m22 - pow(b, 2));
	if (d <= 0.0f)
	{
		d = 1.0f * pow(10.0f, -4);
	}
	
	float e = (m23 - b * c) / d;
	
	float f = sqrt(m33 - pow(c, 2) - pow(e, 2));
	
	if (f <= 0.0f)
	{
		f = 1.0f * pow(10.0f, -4);
	}
	
	float c_1 = z1 / a;
	float c_2 = (z2 - b * c_1) / d;
	float c_3 = (z3 - c * c_1 - e * c_2) / f;
	
	float c3 = c_3 / f;
	float c2 = (c_2 - e * c3) / d;
	float c1 = (c_1 - b * c2 - c * c3) / a;
	
	return vec3(c1, c2, c3);
}

float Shadow(vec4 v, float bias)
{
	vec4 lightDepth = texture(uShadowMap, v.xy);

	vec4 b_ = (1.0f - bias) * lightDepth + bias * vec4(0.5f);
	float pixelDepth = v.z;
	
	vec3 choleskyVec = Cholesky(1.0f, b_.x, b_.y, b_.y, b_.z, b_.w, 
								1.0f, pixelDepth, pixelDepth * pixelDepth);
	
	vec2 roots = Quadratic(choleskyVec.z, choleskyVec.y, choleskyVec.x);

	if (pixelDepth <= roots.x)
	{
		return 0.0f;
	}
	
	else if (pixelDepth <= roots.y)
	{
		float num = (pixelDepth * roots.y - b_[0] * (pixelDepth + roots.y) + b_[1]);
		float denom = (roots.y - roots.x) * (pixelDepth - roots.x);
		
		return (num / denom);
	}
	
	else
	{
		float num = (roots.x * roots.y - b_[0] * (roots.x + roots.y) + b_[1]);
		float denom = (pixelDepth - roots.x) * (pixelDepth - roots.y);
		
		return 1 - (num / denom);
	}
}

vec3 LightCalc()
{
	vec2 fragUV = vec2(gl_FragCoord.x / vWidth, gl_FragCoord.y / vHeight);

	vec3 fragPos = texture(gPos, fragUV).rgb;
	vec3 norm = texture(gNorm, fragUV).rgb;
	vec2 uv = texture(gUVs, fragUV).rg;
	vec3 diff = texture(gAlbedo, fragUV).rgb;
	vec3 specTex = texture(gSpecular, fragUV).rgb;
	float spec = texture(gDepth, fragUV).r;

	// diffuse
	vec3 dir = normalize(-lightDir);
	float nDotL = max(dot(norm, dir), 0.0);
	vec3 finalDiff = nDotL * diff;
	
	// specular
	vec3 viewDir = normalize(viewPos - fragPos);
	
	vec3 reflectDir = reflect(dir, norm);
	float sp = pow(max(dot(viewDir, reflectDir), 0.0), 16.0f);
	vec3 finalSpec = sp * spec * specTex;
	
	vec4 shadowCoord = worldToLightMat * vec4(fragPos, 1.0f);
	vec4 blur = texture(uShadowMap, shadowCoord.xy);
  
	float shadow = Shadow(shadowCoord, 1.0f * pow(10, -3));
	
	float currDepth = shadowCoord.z;
	float maximum = float(currDepth - 1.0f * pow(10, -3) <= blur.x);
	
	return (max(1.0f  - shadow, maximum) * finalDiff) + finalSpec;
}

void main()
{
	vec2 uv = vec2(gl_FragCoord.x / vWidth, 
					gl_FragCoord.y / vHeight);
					
	vec3 localLight = LightCalc();
	
	fragColor = vec4(localLight, 1.0f);
}
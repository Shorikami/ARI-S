#version 430 core

in VS_OUT
{
    vec3 fragDiffuse;
    vec3 fragNormal;
    vec2 texCoords;
    
    vec4 shadowCoord;
    
} fs_in;


out vec4 fragColor;

uniform sampler2D diffTex;
uniform sampler2D uShadowMap;

uniform int vWidth;
uniform int vHeight;

vec2 Quadratic(vec3 v)
{
	float p = v[1] / v[2];
  float q = v[0] / v[2];
  
  float D = (p * p * 0.25f) - q;
  float r = sqrt(D);

	float x1 = -p * 0.5f - r;
	float x2 = -p * 0.5f + r;

	return vec2(x1, x2);
}

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
	
	vec2 roots = Quadratic(choleskyVec.x, choleskyVec.y, choleskyVec.z);
	
	float res = 1.0f;
  
  // Hausdorff
  if (roots.x < 0.0f || roots.y > 1.0f)
  {
    //return 0.0f;
    float zFree = ((b_[2] - b_[1]) * pixelDepth + b_[2] - b_[3]) / ((b_[1] - b_[0]) * pixelDepth + b_[1] - b_[2]);
		
    float w1Factor = (pixelDepth > zFree) ? 1.0f : 0.0f;
		
    res = (b_[1] - b_[0] + (b_[2] - b_[0] - (zFree + 1.0f) * (b_[1] - b_[0])) * (zFree - w1Factor - pixelDepth)
		    / (pixelDepth * (pixelDepth - zFree))) / (zFree - w1Factor) + 1.0f - b_[0];
  }
  
  // Hamburger
  else
  {
    return 0.5f;
  	//vec4 vals = (roots.y < pixelDepth) ? vec4(roots.x, pixelDepth, 1.0f, 1.0f) :
		//((roots.x < pixelDepth) ? vec4(pixelDepth, roots.x, 0.0f, 1.0f) : vec4(0.0f));
    //
    //float quotient = (vals[0] * roots.y - b_[0] * (vals[0] + roots.y) + b_[1]) / ((roots.y - vals[1]) * (pixelDepth - roots[1]));
    //
    //res = vals[2] + vals[3] * quotient;
  }
  
	//// The diffuse part
	//if (pixelDepth <= roots.x)
	//{
	//	res = 0.0f;
	//}
	//
	//// Inside the shadow
	//else if (pixelDepth <= roots.y)
	//{
	//	float num = (pixelDepth * roots.y - b_[0] * (pixelDepth + roots.y) + b_[1]);
	//	float denom = (roots.y - roots.x) * (pixelDepth - roots.x);
	//	
	//	res = roots.x + (num / denom);
	//}
	//
	//// edges of + outside the shadow
	//else
	//{
	//	float num = (roots.x * roots.y - b_[0] * (roots.x + roots.y) + b_[1]);
	//	float denom = (pixelDepth - roots.x) * (pixelDepth - roots.y);
	//	
	//	res = 1 - pixelDepth + 1.0f * (num / denom);
	//}
  
  return 1 - clamp(res, 0.0f, 1.0f);
}

void main()
{
	vec3 lightColor = vec3(1.0f);
	vec3 lightPos = vec3(1.0f);

    // Ambient
	float ambientStrength = 0.9f;
	vec4 ambient = vec4(ambientStrength * vec3(0.0f), 1.0f);
	
	// Diffuse
	vec3 lNormal = normalize(lightPos);
	float dotP = dot(fs_in.fragNormal, lNormal);
	float nDotL = max(dotP, 0.0f);
	vec4 diffuse = vec4(lightColor * nDotL, 1.0f);
  
	float shadow = Shadow(fs_in.shadowCoord, 1.0f * pow(10, -3));
	fragColor = ambient + shadow * diffuse;
}

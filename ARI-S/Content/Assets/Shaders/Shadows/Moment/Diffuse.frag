#version 430 core

in VS_OUT
{
    vec3 fragDiffuse;
    vec3 fragNormal;
} fs_in;


out vec4 fragColor;

uniform sampler2D uShadowMap;

in vec4 shadowCoord;

uniform int vWidth;
uniform int vHeight;

float Det3(vec3 v1, vec3 v2, vec3 v3)
{
	return v1.x * (v2.y * v3.z - v2.z * v3.y) +
		   v1.y * (v2.z * v3.x - v2.x* v3.z) +
		   v1.z * (v2.x * v3.y - v2.y * v3.x);
}

vec3 Cramers(vec3 A, vec3 B, vec3 C, vec3 Z)
{
	float d = Det3(A, B, C);
	float c1 = Det3(Z, B, C) / d;
	float c2 = Det3(A, Z, C) / d;
	float c3 = Det3(A, B, Z) / d;
	
	return vec3(c1, c2, c3);
}

vec2 Quadratic(float a, float b, float c)
{
	float discriminant = b * b - 4 * a * c;
	float x1 = (-b + sqrt(discriminant)) / (2 * a);
	float x2 = (-b - sqrt(discriminant)) / (2 * a);
	
	return vec2(x1, x2);
}

vec2 Quadratic(vec3 v)
{
	float p = v[1] / v[2];
	float q = v[0] / v[2];
	float D = p * p * 0.25f - q;
	float r = sqrt(D);

	float z1 = -p * 0.5f - r;
	float z2 = -p * 0.5f + r;
	return vec2(z1, z2);
}

vec3 Cholesky(vec4 b, float pixelDepth)
{
	float L32D22 = fma(-b[0], b[1], b[2]);
	float D22 = fma(-b[0], b[0], b[1]);
	float sqDepth = fma(-b[1], b[1], b[3]);

	float D33D22 = dot(vec2(sqDepth, -L32D22), vec2(D22, L32D22));
	float InvD22 = 1.0f / D22;
	float L32 = L32D22 * InvD22;

	vec3 c = vec3(1.0f, pixelDepth, pixelDepth * pixelDepth);

	c[1] -= b.x;
	c[2] -= b.y + L32 * c[1];

	c[1] *= InvD22;
	c[2] *= D22 / D33D22;

	c[1] -= L32 * c[2];
	c[0] -= dot(vec2(c.y, c.z), vec2(b.x, b.y));

	return c;
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
	
	return vec3(c3, c2, c1);
}

float Shadow(vec4 v, float bias)
{
	vec4 lightDepth = texture(uShadowMap, v.xy);
  
	vec4 b_ = (1.0f - bias) * lightDepth + bias * vec4(0.5f);
	
	vec3 col1 = vec3(1.0f, b_.x, b_.y);
    vec3 col2 = vec3(b_.x, b_.y, b_.z);
    vec3 col3 = vec3(b_.y, b_.z, b_.w);
	
	float pixelDepth = v.z;
	vec3 rightHand = vec3(1.0f, pixelDepth, pixelDepth * pixelDepth);
	
	//vec3 choleskyVec = Cholesky(1.0f, b_.x, b_.y, b_.y, b_.z, b_.w, 
	//							1.0f, pixelDepth, pixelDepth * pixelDepth);
	//
	//// c3 * z^2 + c2 * z + c1 = 0
	//// x = c3, y = c2, z = c1
	//vec2 roots = Quadratic(choleskyVec.x * pixelDepth * pixelDepth, 
	//					   choleskyVec.y * pixelDepth, 
	//					   choleskyVec.z);
	
	vec3 choleskyVec = Cholesky(b_, pixelDepth);
	
	vec3 cramersVec = Cramers(col1, col2, col3, rightHand);
	vec2 roots = Quadratic(cramersVec.x, cramersVec.y * pixelDepth, cramersVec.z * pixelDepth * pixelDepth);
	
	vec3 ve = vec3(choleskyVec.x, choleskyVec.y, choleskyVec.z);
	roots = Quadratic(ve);
	
	float res = 1.0f;
	
	vec4 vals = (roots.y < pixelDepth) ? vec4(roots.x, pixelDepth, 1.0f, 1.0f) :
		((roots.x < pixelDepth) ? vec4(pixelDepth, roots.x, 0.0f, 1.0f) : vec4(0.0f));

	float quotient = (vals[0] * roots.y - b_[0] * (vals.x + roots.y) + b_[1]) / ((roots.y - vals.y) * (pixelDepth - roots.x));
	
	res = vals.y + vals.z * quotient;
	
	return clamp(res, 0.0f, 1.0f);

	
	//// The diffuse part
	//if (pixelDepth <= roots.x)
	//{
	//	return 0.0f;
	//}
	//
	//// Inside the shadow
	//else if (pixelDepth <= roots.y)
	//{
	//	float num = (pixelDepth * roots.y - b_.x * (pixelDepth + roots.y) + b_.y);
	//	float denom = (roots.y - roots.x) * (pixelDepth - roots.x);
	//	
	//	return (num / denom);
	//}
	//
	//// edges of + outside the shadow
	//else
	//{
	//	float num = (roots.x * roots.y - b_.x * (roots.x + roots.y) + b_.y);
	//	float denom = (pixelDepth - roots.x) * (pixelDepth - roots.y);
	//	
	//	return 1.0f - (num / denom);
	//}
}

void main()
{
	vec3 lightColor = vec3(0.5f);
	vec3 lightPos = vec3(1.0f);

    // Ambient
	float ambientStrength = 0.9f;
	vec4 ambient = vec4(ambientStrength * lightColor, 1.0f);
	
	// Diffuse
	vec3 lNormal = normalize(lightPos);
	float dotP = dot(fs_in.fragNormal, lNormal);
	float nDotL = max(dotP, 0.0f);
	vec4 diffuse = vec4(lightColor * nDotL, 1.0f);
  
	float shadow = Shadow(shadowCoord, 1.0f * pow(10, -3));
  
	if ((1 - shadow) < 0.0f)
	{
		fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if ((1 - shadow) > 1.0f)
	{
		fragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	}
	else
	{
		fragColor = ambient + (1 - shadow) * diffuse;
	}
}

#ifndef CHOLESKY_HPP
#define CHOLESKY_HPP

#include <glm.hpp>

namespace ARIS
{
	glm::vec3 FMA(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		return a * b + c;
	}

	float ShadowTest(float root1, float root2, float depth, glm::vec4 b_)
	{
		// The diffuse part
		if (depth <= root1)
		{
			return 0.0f;
		}

		// Inside the shadow
		else if (depth <= root2)
		{
			float num = (depth * root2 - b_.x * (depth + root2) + b_.y);
			float denom = (root2 - root1) * (depth - root1);

			return (num / denom);
		}

		// edges of + outside the shadow
		else
		{
			float num = (root1 * root2 - b_.x * (root1 + root2) + b_.y);
			float denom = (depth - root1) * (depth - root2);

			return 1.0f - (num / denom);
		}
	}

	float Hamburger(float z, glm::vec2 roots, glm::vec4 b)
	{
		glm::vec4 vals = (roots.y < z) ? glm::vec4(roots.y, roots.x, 1.0f, 1.0f) :
			((roots.x < z) ? glm::vec4(roots.x, roots.y, 0.0f, 1.0f) : glm::vec4(0.0f));

		float quotient = (vals.x * roots.y - b.x * (vals.x + roots.y) + b.y) / ((roots.y - vals.y) * (z - roots.x));
		return vals.z + vals.w * quotient;
	}

	float Hausdorff(float z, glm::vec2 roots, glm::vec4 b)
	{
		float res = 1.0f;
		if(roots.x < 0.0f || roots.y > 1.0f)
		{
		    float zFree = ((b[2] - b[1]) * z + b[2] - b[3]) / ((b[1] - b[0]) * z + b[1] - b[2]);
		    float w1Factor = (z > zFree) ? 1.0f : 0.0f;
		    res = (b[1] - b[0] + (b[2] - b[0] - (zFree + 1.0f) * (b[1] - b[0])) * (zFree - w1Factor - z)
		    / (z * (z - zFree))) / (zFree - w1Factor) + 1.0f - b[0];
		}

		// Use the solution with three deltas
		else    
		{
			glm::vec4 vals = (roots.y < z) ? glm::vec4(roots.y, roots.x, 1.0f, 1.0f) :
				((roots.x < z) ? glm::vec4(roots.x, roots.y, 0.0f, 1.0f) : glm::vec4(0.0f));

			float quotient = (vals.x * roots.y - b.x * (vals.x + roots.y) + b.y) / ((roots.y - vals.y) * (z - roots.x));
			res = vals.z + vals.w * quotient;
		}

		return res;
	}

	float Det3(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
	{
		return v1.x * (v2.y * v3.z - v2.z * v3.y) +
			   v1.y * (v2.z * v3.x - v2.x* v3.z) +
			   v1.z * (v2.x * v3.y - v2.y * v3.x);
	}

	static glm::vec2 Quadratic(float a, float b, float c)
	{
		float discriminant = b * b - 4 * a * c;
		float t = sqrtf(discriminant);

		float x1 = (-b + t) / (2 * a);
		float x2 = (-b - t) / (2 * a);

		if (x1 > x2)
		{
			return glm::vec2(x2, x1);
		}
		return glm::vec2(x1, x2);
	}

	static glm::vec2 Quadratic(glm::vec3 v)
	{
		float p = v[1] / v[2];
		float q = v[0] / v[2];
		float D = p * p * 0.25f - q;
		float r = sqrt(D);

		float z1 = -p * 0.5f - r;
		float z2 = -p * 0.5f + r;

		if (z1 > z2)
		{
			return glm::vec2(z2, z1);
		}

		return glm::vec2(z1, z2);
	}

	static glm::vec3 Cramers(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 Z)
	{
		float d = Det3(A, B, C);
		float c1 = Det3(Z, B, C) / d;
		float c2 = Det3(A, Z, C) / d;
		float c3 = Det3(A, B, Z) / d;
		
		return glm::vec3(c1, c2, c3);
	}

	static glm::vec3 Cholesky(float m11, float m12, float m13, float m22, float m23, float m33,
		float z1, float z2, float z3)
	{
		float a = sqrtf(m11);
		if (a <= 0.0f)
		{
			a = 1.0f * powf(10.0f, -4);
		}

		float b = m12 / a;
		float c = m13 / a;

		float d = sqrtf(m22 - powf(b, 2));

		if (d <= 0.0f)
		{
			d = 1.0f * powf(10.0f, -4);
		}

		float e = (m23 - b * c) / d;

		float f = sqrtf(m33 - powf(c, 2) - powf(e, 2));

		if (f <= 0.0f)
		{
			f = 1.0f * powf(10.0f, -4);
		}

		float c_1 = z1 / a;
		float c_2 = (z2 - b * c_1) / d;
		float c_3 = (z3 - c * c_1 - e * c_2) / f;

		float c3 = c_3 / f;
		float c2 = (c_2 - e * c3) / d;
		float c1 = (c_1 - b * c2 - c * c3) / a;

		return glm::vec3(c1, c2, c3);
	}

	static glm::vec3 Cholesky(glm::vec4 b, float pixelDepth)
	{
		float L32D22 = fmaf(-b[0], b[1], b[2]);
		float D22 = fmaf(-b[0], b[0], b[1]);
		float sqDepth = fmaf(-b[1], b[1], b[3]);

		float D33D22 = glm::dot(glm::vec2(sqDepth, -L32D22), glm::vec2(D22, L32D22));
		float InvD22 = 1.0f / D22;
		float L32 = L32D22 * InvD22;

		glm::vec3 c = glm::vec3(1.0f, pixelDepth, pixelDepth * pixelDepth);

		c[1] -= b.x;
		c[2] -= b.y + L32 * c[1];

		c[1] *= InvD22;
		c[2] *= D22 / D33D22;

		c[1] -= L32 * c[2];
		c[0] -= glm::dot(glm::vec2(c.y, c.z), glm::vec2(b.x, b.y));

		return c;
	}
}

#endif
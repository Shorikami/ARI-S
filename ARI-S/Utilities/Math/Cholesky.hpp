#ifndef CHOLESKY_HPP
#define CHOLESKY_HPP

#include <glm.hpp>

namespace ARIS
{
	float Det3(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
	{
		return v1.x * (v2.y * v3.z - v2.z * v3.y) +
			   v1.y * (v2.z * v3.x - v2.x* v3.z) +
			   v1.z * (v2.x * v3.y - v2.y * v3.x);
	}

	static glm::vec2 Quadratic(float a, float b, float c)
	{
		float discriminant = b * b - 4 * a * c;
		float x1 = (-b + sqrt(discriminant)) / (2 * a);
		float x2 = (-b - sqrt(discriminant)) / (2 * a);

		return glm::vec2(x1, x2);
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

		return glm::vec3(c3, c2, c1);
	}
}

#endif
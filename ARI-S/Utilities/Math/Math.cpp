#include "arpch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/matrix_decompose.hpp>

namespace ARIS::Math
{
	bool Decompose(const glm::mat4& transf, glm::vec3& tr, glm::vec3& ro, glm::vec3& sc)
	{
		glm::mat4 LocalMatr(transf);

		if (glm::epsilonEqual(LocalMatr[3][3], static_cast<float>(0), glm::epsilon<float>()))
		{
			return false;
		}

		if (
			glm::epsilonNotEqual(LocalMatr[0][3], static_cast<float>(0), glm::epsilon<float>()) ||
			glm::epsilonNotEqual(LocalMatr[1][3], static_cast<float>(0), glm::epsilon<float>()) ||
			glm::epsilonNotEqual(LocalMatr[2][3], static_cast<float>(0), glm::epsilon<float>()))
		{
			LocalMatr[0][3] = LocalMatr[1][3] = LocalMatr[2][3] = static_cast<float>(0);
			LocalMatr[3][3] = static_cast<float>(1);
		}

		// Next take care of translation (easy).
		tr = glm::vec3(LocalMatr[3]);
		LocalMatr[3] = glm::vec4(0, 0, 0, LocalMatr[3].w);

		glm::vec3 Row[3], Pdum3;

		// Now get scale and shear.
		for (glm::length_t i = 0; i < 3; ++i)
			for (glm::length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatr[i][j];

		// Compute X scale factor and normalize first row.
		sc.x = length(Row[0]);
		Row[0] = glm::detail::scale(Row[0], static_cast<float>(1));
		sc.y = length(Row[1]);
		Row[1] = glm::detail::scale(Row[1], static_cast<float>(1));
		sc.z = length(Row[2]);
		Row[2] = glm::detail::scale(Row[2], static_cast<float>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
#if 0
		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

		ro.y = asin(-Row[0][2]);
		if (cos(ro.y) != 0) {
			ro.x = atan2(Row[1][2], Row[2][2]);
			ro.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			ro.x = atan2(-Row[2][0], Row[1][1]);
			ro.z = 0;
		}


		return true;
	}
}
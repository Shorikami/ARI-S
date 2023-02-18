#ifndef MATH_H
#define MATH_H

#include <glm.hpp>

namespace ARIS::Math
{
	bool Decompose(const glm::mat4& transf, glm::vec3& tr, glm::vec3& ro, glm::vec3& sc);
}

#endif
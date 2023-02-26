#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>

namespace ARIS
{
	class Camera
	{
	public:
		enum class CameraDirection
		{
			NONE = 0,
			FORWARD,
			BACKWARDS,
			LEFT,
			RIGHT,
			UP,
			DOWN
		};

	public:
		Camera() = default;
		Camera(const glm::mat4& proj)
			: m_Projection(proj)
		{
		}

		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }

	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};
}


#endif
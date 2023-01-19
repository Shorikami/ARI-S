#include "Camera.h"
#include <cmath>

namespace Hayase
{
	Camera::Camera(glm::vec3 pos)
		: cameraPos(pos)
		, yaw(0.0f)
		, pitch(0.0f)
		, speed(2.5f)
		, zoom(45.0f)
		, front(glm::vec3(0.0f, 0.0f, -1.0f))
		, n(0.1f)
		, f(100.0f)
	{
		UpdateCameraVectors();
	}

	void Camera::UpdateCameraDir(double dx, double dy)
	{
		if (rotateCamera)
		{
			yaw += dx;
			pitch += dy;

			if (pitch > 89.0f)
			{
				pitch = 89.0f;
			}
			else if (pitch < -89.0f)
			{
				pitch = -89.0f;
			}

			UpdateCameraVectors();
		}
	}

	void Camera::UpdateCameraPos(CameraDirection dir, double dt)
	{
		float velocity = (float)dt * speed;

		switch (dir)
		{
		case CameraDirection::FORWARD:
			cameraPos += front * velocity;
			break;
		case CameraDirection::BACKWARDS:
			cameraPos -= front * velocity;
			break;
		case CameraDirection::RIGHT:
			cameraPos += glm::normalize(glm::cross(front, up)) * velocity;
			break;
		case CameraDirection::LEFT:
			cameraPos -= glm::normalize(glm::cross(front, up)) * velocity;
			break;
		}
	}

	void Camera::UpdateCameraZoom(double dy)
	{
		if (zoom >= 1.0f && zoom <= 45.0f)
		{
			zoom -= dy;
		}
		else if (zoom < 1.0f)
		{
			zoom = 1.0f;
		}
		else
		{
			zoom = 45.0f;
		}
	}

	void Camera::UpdateCameraVectors()
	{
		glm::vec3 dir;
		dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		dir.y = sin(glm::radians(pitch));
		dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(dir);

		right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		up = glm::normalize(glm::cross(right, front));
	}


	//----------------------------------
	//----------------------------------
	// CS250 CAMERA
	//----------------------------------
	//----------------------------------

	CS250_Camera::CS250_Camera(void)
	{
		eye_point = glm::vec4(glm::vec3(0.0f), 1.0f);

		right_vector = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		back_vector = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

		vp_distance = 5.0f;
		vp_width = 2.0f * vp_distance * tanf((90.0f / 2.0f) * (glm::pi<float>() / 180.0f));
		vp_height = vp_width / 1.0f;

		near_distance = 0.1f;
		far_distance = 10.0f;
	}

	CS250_Camera::CS250_Camera(const glm::vec4& E, const glm::vec4& look, const glm::vec4& rel, float fov, float aspect, float N, float F)
		: eye_point(E)
		, back_vector(glm::normalize(look * -1.0f))
		, right_vector(glm::normalize(glm::vec4(glm::cross(glm::vec3(look), glm::vec3(rel)), 0.0f)))
		, up_vector(glm::normalize(rel))
		, near_distance(N)
		, far_distance(F)
		, vp_distance(N)
	{
		float rad = glm::radians(fov / 2.0f);
		vp_width = tanf(rad) * vp_distance * 2.0f;
		vp_height = vp_width / aspect;
	}

	glm::vec4 CS250_Camera::viewport(void) const
	{
		return glm::vec4(vp_width, vp_height, vp_distance, 0.0f);
	}

	CS250_Camera& CS250_Camera::zoom(float factor)
	{
		vp_width *= factor;
		vp_height *= factor;
		return *this;
	}

	CS250_Camera& CS250_Camera::forward(float distance)
	{
		eye_point -= back_vector * distance;
		return *this;
	}

	CS250_Camera& CS250_Camera::yaw(float angle)
	{
		right_vector = rotate(angle, up_vector) * right_vector;
		back_vector = rotate(angle, up_vector) * back_vector;
		return *this;
	}

	CS250_Camera& CS250_Camera::pitch(float angle)
	{
		up_vector = rotate(angle, right_vector) * up_vector;
		back_vector = rotate(angle, right_vector) * back_vector;
		return *this;
	}

	CS250_Camera& CS250_Camera::roll(float angle)
	{
		right_vector = rotate(angle, back_vector) * right_vector;
		up_vector = rotate(angle, back_vector) * up_vector;
		return *this;
	}

	glm::mat4 CS250_Camera::rotate(float deg, const glm::vec4& v)
	{
		glm::mat3 outerProduct = { {v.x * v.x, v.y * v.x, v.z * v.x},
							  {v.x * v.y, v.y * v.y, v.z * v.y},
							  {v.x * v.z, v.y * v.z, v.z * v.z} };
		glm::mat3 crossProduct = { {0, v.z, -v.y},
								  {-v.z, 0, v.x},
								  {v.y, -v.x, 0} };

		float vecMag = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));

		float cosDeg = cos(glm::radians(deg));
		float sinDeg = sin(glm::radians(deg));

		glm::mat3 result = (cosDeg * glm::mat3(1)) +
			((1 - cosDeg) / (vecMag * vecMag)) * outerProduct +
			(sinDeg / vecMag) * crossProduct;

		return glm::mat4(result);
	}

	glm::mat4 CS250_Camera::affine(const glm::vec4& Lx, const glm::vec4& Ly, const glm::vec4& Lz, const glm::vec4& C)
	{
		glm::mat4 result = { Lx, Ly, Lz, C };

		return result;
	}

	glm::mat4 CS250_Camera::affineInv(const glm::mat4& A)
	{
		glm::mat3 linearTransformation = glm::mat3(A);
		glm::vec3 translation = glm::vec3(A[3]);

		linearTransformation = glm::inverse(linearTransformation);
		translation *= -1.0f;

		glm::mat4 result = glm::mat4(linearTransformation);
		result[3] = glm::vec4(linearTransformation * translation, 1.0f);

		return result;
	}

	glm::mat4 CS250_Camera::model()
	{
		return affine(right_vector, up_vector, back_vector, eye_point);
	}

	glm::mat4 CS250_Camera::view()
	{
		return affineInv(model());
	}

	glm::mat4 CS250_Camera::perspective()
	{
		float W = vp_width;
		float H = vp_height;
		float D = vp_distance;

		// Identity
		glm::mat4 matrix(1);


		matrix[0][0] = (2 * D) / W;
		matrix[1][1] = (2 * D) / H;
		matrix[2][2] = (near_distance + far_distance) / (near_distance - far_distance);
		matrix[3][3] = 0;

		matrix[2][3] = -1;
		matrix[3][2] = (2 * near_distance * far_distance) / (near_distance - far_distance);

		return matrix;
	}
}
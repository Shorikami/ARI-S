#ifndef ARVECTOR_H
#define ARVECTOR_H

#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>

namespace ARIS
{
	struct Vector2
	{
		float x;
		float y;

		Vector2(float x_ = 0.0f, float y_ = 0.0f)
			: x(x_)
			, y(y_)
		{
		}

		Vector2 operator+(Vector2 const& other)
		{
			this->x += other.x;
			this->y += other.y;
			return *this;
		}

		static glm::vec2 ToVecGLM(Vector2& v) { return glm::vec2(v.x, v.y); }
	};

	

	struct Vector3
	{
		float x;
		float y;
		float z;

		Vector3(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f)
			: x(x_)
			, y(y_)
			, z(z_)
		{
		}

		Vector3(glm::vec3 v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
		}

		static glm::vec3 ToVecGLM(Vector3& v) { return glm::vec3(v.x, v.y, v.z); }


		template <typename T>
		static glm::vec3 ToVecGLM(T v) { return glm::vec3(0.0f); }
	};

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;

		Vector4(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f, float w_ = 0.0f)
			: x(x_)
			, y(y_)
			, z(z_)
			, w(w_)
		{
		}

		Vector4(glm::vec4 v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
		}

		static glm::vec4 ToVecGLM(Vector4& v) { return glm::vec4(v.x, v.y, v.z, v.w); }


		template <typename T>
		static glm::vec4 ToVecGLM(T v) { return glm::vec4(1.0f); }
	};

	struct Matrix4
	{
		float x[4];
		float y[4];
		float z[4];
		float w[4];

		// Identity by default
		Matrix4()
		{
			x[0] = y[1] = z[2] = w[3] = 1.0f;
			x[1] = x[2] = x[3] = 0.0f;
			y[0] = y[2] = y[3] = 0.0f;
			z[0] = z[1] = z[3] = 0.0f;
			w[0] = w[1] = w[2] = 0.0f;
		}

		Matrix4(float x_[4], float y_[4], float z_[4], float w_[4])
		{
			x[0] = x_[0];
		}

		Matrix4(glm::mat4 m)
		{
			x[0] = m[0][0];
			x[1] = m[0][1];
			x[2] = m[0][2];
			x[3] = m[0][3];

			y[0] = m[1][0];
			y[1] = m[1][1];
			y[2] = m[1][2];
			y[3] = m[1][3];

			z[0] = m[2][0];
			z[1] = m[2][1];
			z[2] = m[2][2];
			z[3] = m[2][3];

			w[0] = m[3][0];
			w[1] = m[3][1];
			w[2] = m[3][2];
			w[3] = m[3][3];
		}

		static glm::mat4 ToMatGLM(Matrix4& v) 
		{ 
			float res[16] = 
			{ 
				v.x[0], v.y[0], v.z[0], v.w[0],
				v.x[1], v.y[1], v.z[1], v.w[1],
				v.x[2], v.y[2], v.z[2], v.w[2],
				v.x[3], v.y[3], v.z[3], v.w[3] 
			};

			return glm::make_mat4(res);
		}

		template <typename T>
		static glm::mat4 ToMatGLM(T v) { return glm::mat4(1.0f); }
	};

	std::ostream& operator<<(std::ostream& os, const Vector3& v)
	{
		os << "{ " << v.x << ", " << v.y << ", " << v.z << " }";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Vector4& v)
	{
		os << "{ " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " }";
		return os;
	}
}

#endif
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>

namespace ARIS
{

	class Shader
	{
	public:
		unsigned int m_ID;
		std::string m_VertSrc, m_GeoSrc, m_FragSrc;

		Shader();
		Shader(bool includeDefaultHeader, const char* vertPath, const char* fragPath, const char* geoPath = nullptr);

		void Generate(bool includeDefaultHeader, const char* vPath, const char* fPath, const char* gPath = nullptr);
		void Activate();


		GLuint Compile(bool includeDefaultHeader, const char* path, GLenum type, std::string& src);

		void SetBool(const std::string& name, bool val);

		void SetInt(const std::string& name, int val);
		
		template <typename U>
		void SetInt(const std::string& name, U val)
		{
		}

		void SetFloat(const std::string& name, float val);

		template <typename U>
		void SetFloat(const std::string& name, U val)
		{
		}

		void SetVec3(const std::string& name, float v1, float v2, float v3);
		void SetVec3(const std::string& name, glm::vec3 v);

		void SetVec4(const std::string& name, float v1, float v2, float v3, float v4);
		void SetVec4(const std::string& name, glm::vec4 v);

		void SetMat4(const std::string& name, glm::mat4 val);

		template<typename T>
		void SetData(const std::string& name, T t)
		{
			if (std::is_same<T, bool>::value || std::is_same<T, int>::value)
			{
				SetInt(name, t);
			}
			else if (std::is_same<T, float>::value)
			{
				SetFloat(name, t);
			}
			else if (std::is_same<T, Vector3>::value)
			{
				SetVec3(name, Vector3::ToVecGLM(t));
			}
			else if (std::is_same<T, Vector4>::value)
			{
				SetVec4(name, Vector4::ToVecGLM(t));
			}
			else if (std::is_same<T, Matrix4>::value)
			{
				SetMat4(name, Matrix4::ToMatGLM(t));
			}
		}

		static std::string defaultDirectory;
		static std::stringstream defaultHeaders;

		static void LoadIntoDefault(const char* path);
		static void ClearDefault();

		static std::string LoadShaderSrc(bool includeDefaultHeader, const char* path);
	};
}

#endif
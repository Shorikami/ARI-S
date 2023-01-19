#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Hayase
{
	class Shader
	{
	public:
		unsigned int m_ID;

		Shader();
		Shader(bool includeDefaultHeader, const char* vertPath, const char* fragPath, const char* geoPath = nullptr);

		void Generate(bool includeDefaultHeader, const char* vPath, const char* fPath, const char* gPath = nullptr);
		void Activate();


		GLuint Compile(bool includeDefaultHeader, const char* path, GLenum type);

		void SetBool(const std::string& name, bool val);

		void SetInt(const std::string& name, int val);
		void SetFloat(const std::string& name, float val);

		void SetVec3(const std::string& name, float v1, float v2, float v3);
		void SetVec3(const std::string& name, glm::vec3 v);

		void SetVec4(const std::string& name, float v1, float v2, float v3, float v4);
		void SetVec4(const std::string& name, glm::vec4 v);
		//void SetVec4(const std::string& name, aiColor4D color);

		void SetMat4(const std::string& name, glm::mat4 val);

		static std::string defaultDirectory;
		static std::stringstream defaultHeaders;

		static void LoadIntoDefault(const char* path);
		static void ClearDefault();

		static std::string LoadShaderSrc(bool includeDefaultHeader, const char* path);
	};
}

#endif
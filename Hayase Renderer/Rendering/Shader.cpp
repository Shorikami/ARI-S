#include "Shader.h"

namespace Hayase
{
	Shader::Shader()
	{
	}

	Shader::Shader(bool include, const char* vPath, const char* fPath, const char* gPath)
	{
		Generate(include, vPath, fPath, gPath);
	}

	void Shader::Generate(bool include, const char* vPath, const char* fPath, const char* gPath)
	{
		int success;
		char infoLog[512];

		GLuint vShader = Compile(include, vPath, GL_VERTEX_SHADER);
		GLuint fShader = Compile(include, fPath, GL_FRAGMENT_SHADER);

		GLuint gShader = 0;
		if (gPath)
		{
			gShader = Compile(include, gPath, GL_GEOMETRY_SHADER);
		}

		m_ID = glCreateProgram();
		glAttachShader(m_ID, vShader);
		glAttachShader(m_ID, fShader);
		if (gPath)
		{
			glAttachShader(m_ID, gShader);
		}
		glLinkProgram(m_ID);

		glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
			std::cout << "Program linking error: " << std::endl << infoLog << std::endl;
		}

		glDeleteShader(vShader);
		glDeleteShader(fShader);
		if (gPath)
		{
			glDeleteShader(gShader);
		}
	}

	void Shader::Activate()
	{
		glUseProgram(m_ID);
	}

	std::stringstream Shader::defaultHeaders;

	void Shader::LoadIntoDefault(const char* path)
	{
		std::string contents = Shader::LoadShaderSrc(false, path);
		Shader::defaultHeaders << contents;
	}

	void Shader::ClearDefault()
	{
		Shader::defaultHeaders.clear();
	}

	std::string Shader::LoadShaderSrc(bool include, const char* name)
	{
		std::ifstream file;
		std::stringstream buf;
		std::string res = "";

		if (include)
		{
			buf << Shader::defaultHeaders.str();
		}

		std::string fullPath = Shader::defaultDirectory + '/' + name;

		file.open(fullPath);

		if (file.is_open())
		{
			buf << file.rdbuf();
			res = buf.str();
		}
		else
		{
			std::cout << "Could not open " << name << std::endl;
		}

		file.close();

		return res;
	}

	GLuint Shader::Compile(bool include, const char* path, GLenum type)
	{
		int success;
		char infoLog[512];

		GLuint res = glCreateShader(type);
		std::string src = LoadShaderSrc(include, path);
		const GLchar* shader = src.c_str();

		glShaderSource(res, 1, &shader, NULL);
		glCompileShader(res);

		glGetShaderiv(res, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(res, 512, NULL, infoLog);
			std::cout << "Error with shader from " << path << ": " << std::endl << infoLog << std::endl;
		}

		return res;
	}

	void Shader::SetBool(const std::string& name, bool val)
	{
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int)val);
	}

	void Shader::SetInt(const std::string& name, int val)
	{
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), val);
	}

	void Shader::SetFloat(const std::string& name, float val)
	{
		glUniform1f(glGetUniformLocation(m_ID, name.c_str()), val);
	}

	void Shader::SetVec3(const std::string& name, float v1, float v2, float v3)
	{
		glUniform3f(glGetUniformLocation(m_ID, name.c_str()), v1, v2, v3);
	}

	void Shader::SetVec3(const std::string& name, glm::vec3 v)
	{
		glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, &v[0]);
	}

	void Shader::SetVec4(const std::string& name, float v1, float v2, float v3, float v4)
	{
		glUniform4f(glGetUniformLocation(m_ID, name.c_str()), v1, v2, v3, v4);
	}

	void Shader::SetVec4(const std::string& name, glm::vec4 v)
	{
		glUniform4fv(glGetUniformLocation(m_ID, name.c_str()), 1, &v[0]);
	}

	//void Shader::SetVec4(const std::string& name, aiColor4D color)
	//{
	//	glUniform4f(glGetUniformLocation(m_ID, name.c_str()), color.r, color.g, color.b, color.a);
	//}

	void Shader::SetMat4(const std::string& name, glm::mat4 val)
	{
		glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
	}
}

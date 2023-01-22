#ifndef UNIFORMMEMORY_HPP
#define UNIFORMMEMORY_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#define NUM_LIGHTS 1
#define MAX_LIGHTS 200 // breaks after going beyond 32 because ubo info is passed in incorrectly, pls fix later

namespace Hayase
{
	template <typename T>
	class UniformBuffer
	{
	public:
		UniformBuffer(unsigned idx)
		{
			glGenBuffers(1, &id);

			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glBindBufferBase(GL_UNIFORM_BUFFER, idx, id);
		}

		void SetData()
		{
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &data);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void UpdateData(GLintptr offset)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(T) - offset, static_cast<char*>(&data) + offset);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		T& GetData()
		{
			return data;
		}

	private:
		GLuint id;
		T data;
	};

	class World
	{
	public:
		glm::mat4 proj = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::vec2 nearFar = glm::vec2(0.0f);
	};

	class Lights
	{
	public:
		glm::vec4 lightPos[MAX_LIGHTS] = {};
		glm::vec4 lightColor[MAX_LIGHTS] = {};
		glm::vec4 lightDir[MAX_LIGHTS] = {};

		glm::vec4 eyePos = {};
		glm::vec4 emissive = {};
		glm::vec4 globalAmbient = {};
		glm::vec4 coefficients = {}; // x = kA, y = kD, z = kS, w = ns

		glm::vec4 fogColor = glm::vec4(1.0f);

		glm::vec4 specular[MAX_LIGHTS] = {};
		glm::vec4 ambient[MAX_LIGHTS] = {};
		glm::vec4 diffuse[MAX_LIGHTS] = {};

		glm::vec4 lightInfo[MAX_LIGHTS] = {}; // x = inner, y = outer, z = falloff, w = type

		glm::ivec4 modes = {}; // x = use gpu, y = use normals, z = UV calculation type

		glm::vec3 attenuation = glm::vec3(0.5f, 0.37f, 0.2f); // x = c1, y = c2, z = c3
		int numLights;
		//float _pad; // std140 requires padding - vec4 = 16 bytes, vec3 + float == 12 + 4 = 16 bytes
	};

	class LocalLight
	{
	public:
		glm::vec4 pos = glm::vec4(glm::vec3(0.0f), 1.0f); // xyz = pos, w = range
		glm::vec4 color = glm::vec4(1.0f);
		glm::vec4 options = glm::vec4(1.0f); // intensity, cutoff, max range
	};
}

#endif
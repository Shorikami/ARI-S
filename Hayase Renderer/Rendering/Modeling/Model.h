#ifndef MODEL_H
#define MODEL_H

#include "VertexMemory.hpp"
#include <glm.hpp>
#include <vector>

namespace Hayase
{
	class ModelBuilder;
	class Texture;

	struct Vertex
	{
		glm::vec3 s_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 s_Normal = { 0.0f, 0.0f, 0.0f };
		glm::vec2 s_UV = { 0.0f, 0.0f };
		glm::vec3 s_Tangent = { 0.0f, 0.0f, 0.0f };

		bool operator==(const Vertex& other) const
		{
			return s_Position == other.s_Position &&
				   s_Normal == other.s_Normal &&
				   s_UV == other.s_UV &&
				   s_Tangent == other.s_Tangent;
		}
	};

	class Model
	{
	public:
		Model() = default;
		Model(const Model& other);
		Model(std::string path);

		void BuildArrays();
		void DestroyArrays();

		std::string GetName() const { return m_Name; }

		VertexArray GetVAO() const { return m_VertexArray; }

	private:

		std::string m_Name;

		std::vector<Vertex> m_VertexData;
		std::vector<glm::vec3> m_Vertices;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec2> m_UVs;
		std::vector<unsigned int> m_Indices;

		VertexArray m_VertexArray;
		//VertexArray m_NormalArray;
		//VertexArray m_FaceArray;

		friend class ModelBuilder;
	};
}

#endif
#ifndef MODEL_H
#define MODEL_H

#include "VertexMemory.hpp"
#include <glm.hpp>
#include <vector>

namespace ARIS
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

		void operator=(const Model& other);

		void BuildArrays();
		void InitializeID(int entityID);

		void DestroyArrays();

		std::string GetName() const { return m_Name; }

		VertexArray GetVAO() const& { return m_VertexArray; }

		size_t GetVertexCount() { return m_Vertices.size(); }
		size_t GetIndexCount() { return m_Indices.size(); }

		size_t GetNormalCount() { return m_Normals.size(); }
		size_t GetTexCoordCount() { return m_UVs.size(); }

		std::vector<glm::vec3> GetVertices() const { return m_Vertices; }
		std::vector<unsigned int> GetIndices() const { return m_Indices; }

		std::vector<glm::vec3> GetNormals() const { return m_Normals; }
		std::vector<glm::vec2> GetTexCoords() const { return m_UVs; }

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
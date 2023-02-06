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
		Model(std::string path);

		void BuildMeshArray();

		VertexArray GetMeshArray() const { return m_MeshArray; }
	private:

		std::vector<Vertex> m_Vertices;
		std::vector<unsigned int> m_Indices;

		VertexArray m_MeshArray;

		friend class ModelBuilder;
	};

	class ModelBuilder
	{
	public:
		static void LoadOBJ(std::string path, Model& model);
		static void LoadGLTF(std::string path, Model& model);

		std::unordered_map<std::string, Model> GetModelTable() { return m_ModelTable; }

	private:
		static std::unordered_map<std::string, Model> m_ModelTable;
	};
}

#endif
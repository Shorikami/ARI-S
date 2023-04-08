#ifndef MODEL_H
#define MODEL_H

#include "VertexMemory.hpp"
#include "Mesh.h"
#include "Shader.h"

#include <glm.hpp>
#include <vector>

namespace ARIS
{
	class Texture;
	class ModelBuilder;

	class Model
	{
	public:
		Model() = default;
		Model(const Model& other);
		Model(std::string path);

		void operator=(const Model& other);

		void InitializeID(int entityID);

		void Draw(Shader& shader, int entID = -1);

		std::string GetName() const { return m_Name; }
		std::string GetPath() const { return m_Path; }

		void SetName(std::string s) { m_Name = s; }
		void SetPath(std::string s) { m_Path = s; }

		std::vector<Mesh> GetMeshes() const { return m_Meshes; }
		std::vector<Texture> GetLoadedTextures() const { return m_LoadedTextures; }

	private:
		std::vector<Texture> m_LoadedTextures;
		std::vector<Mesh> m_Meshes;

		std::string m_Name;
		std::string m_Path;

		friend class ModelBuilder;
		friend class SceneSerializer;
		friend class HierarchyPanel;
	};
}

#endif
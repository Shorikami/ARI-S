#ifndef MODELBUILDER_H
#define MODELBUILDER_H

#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace ARIS
{
	class ModelBuilder
	{
		typedef std::pair<glm::vec3, int> VertexPair;

	public:
		struct Compare
		{
			float eps = 0.00001f;

			bool operator() (const VertexPair& p1, const VertexPair& p2) const
			{
				if (fabs(p1.first.x - p2.first.x) > eps) return p1.first.x < p2.first.x;
				if (fabs(p1.first.y - p2.first.y) > eps) return p1.first.y < p2.first.y;
				if (fabs(p1.first.z - p2.first.z) > eps) return p1.first.z < p2.first.z;
				return false;
			}
		};

		ModelBuilder();
		~ModelBuilder();

		void DestroyTable();

		Model* LoadModel(std::string path);

		inline static ModelBuilder& Get() { return *m_Instance; }
		
		std::vector<Model*> GetModelTable() { return m_ModelTable; }

		static Model* CreateSphere(float radius, unsigned divisions);

	private:
		void GenerateModel(std::string path, Model& model);
		void ProcessNode(aiNode* node, const aiScene* scene, Model& model);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, Model& model);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, Model& model);

		std::vector<Model*> m_ModelTable;
		static ModelBuilder* m_Instance;
	};
}

#endif
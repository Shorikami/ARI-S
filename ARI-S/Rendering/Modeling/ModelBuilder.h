#ifndef MODELBUILDER_H
#define MODELBUILDER_H

#include "Model.h"

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

		void BuildTable();
		void DestroyTable();

		void LoadOBJ(std::string path, Model& model);
		void LoadGLTF(std::string path, Model& model);

		inline static ModelBuilder& Get() { return *m_Instance; }
		
		std::unordered_map<std::string, Model*> GetModelTable() { return m_ModelTable; }

		static void CreateSphere(float radius, unsigned divisions, Model& model);
		static void CreateFrustum(Model& model);

	private:
		void BuildNormals(Model& m);
		void BuildTexCoords(Model& m);

		std::unordered_map<std::string, Model*> m_ModelTable;
		static ModelBuilder* m_Instance;
	};
}

#endif
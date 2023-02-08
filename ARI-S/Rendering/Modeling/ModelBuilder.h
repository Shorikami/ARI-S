#ifndef MODELBUILDER_H
#define MODELBUILDER_H

#include "Model.h"

namespace ARIS
{
	class ModelBuilder
	{
	public:
		ModelBuilder();
		~ModelBuilder();

		void BuildTable();
		void DestroyTable();

		void LoadOBJ(std::string path, Model& model);
		void LoadGLTF(std::string path, Model& model);

		inline static ModelBuilder& Get() { return *m_Instance; }
		
		std::unordered_map<std::string, Model*> GetModelTable() { return m_ModelTable; }

		static void CreateSphere(float radius, unsigned divisions, Model& model);

	private:
		void BuildNormals(Model& m);
		void BuildTexCoords(Model& m);

		std::unordered_map<std::string, Model*> m_ModelTable;
		static ModelBuilder* m_Instance;
	};
}

#endif
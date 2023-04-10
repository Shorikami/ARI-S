#include <arpch.h>
#include "Model.h"
#include "ModelBuilder.h"

#include "Texture.h"

namespace ARIS
{
    Model::Model(const Model& other)
      : m_Name(other.m_Name)
      , m_Path(other.m_Path)
      , m_Meshes(other.m_Meshes)
      , m_LoadedTextures(other.m_LoadedTextures)
    {
        for (Mesh& m : m_Meshes)
        {
            m.m_Textures.clear();
        }
    }

    void Model::operator=(const Model& other)
    {
        m_Name = other.m_Name;
        m_Path = other.m_Path;
        m_Meshes = other.m_Meshes;
        m_LoadedTextures = other.m_LoadedTextures;
    }

	Model::Model(std::string path)
        : m_Name(std::string())
        , m_Path(path)
	{
        ModelBuilder::Get().LoadModel(path);
	}

    void Model::InitializeID(int id)
    {
        for (Mesh m : m_Meshes)
        {
            std::vector<int> data(m.GetVertexData().size(), id);
            m.GetVAO().Bind();

           m.GetVAO()["EntityID"] = VertexBuffer(GL_ARRAY_BUFFER);
           m.GetVAO()["EntityID"].Generate();
           m.GetVAO()["EntityID"].Bind();
           m.GetVAO()["EntityID"].SetData<GLint>(m.GetVertexData().size(), data.data(), GL_STATIC_DRAW);
           m.GetVAO()["EntityID"].SetAttPointer<GLint>(3, 1, GL_INT, 1, 0);
           m.GetVAO()["EntityID"].Unbind();
           m.GetVAO().Clear();
        }
    }

    void Model::Update(glm::mat4 modelMat)
    {
        for (unsigned i = 0; i < m_Meshes.size(); ++i)
        {
            m_Meshes[i].Update(modelMat);
        }
    }

    void Model::Draw(Shader& shader, int entID)
    {
        for (unsigned i = 0; i < m_Meshes.size(); ++i)
        {
            m_Meshes[i].Draw(shader, entID);

            if (ModelBuilder::Get().m_DisplayBoxes)
                m_Meshes[i].DrawBoundingBox();
        }
    }
}
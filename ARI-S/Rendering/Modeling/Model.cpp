#include <arpch.h>
#include "Model.h"
#include "ModelBuilder.h"

#include "Texture.h"

namespace ARIS
{
    Model::Model(const Model& other)
      : m_Name(other.m_Name)
      , m_VertexData(other.m_VertexData)
      , m_Vertices(other.m_Vertices)
      , m_Normals(other.m_Normals)
      , m_UVs(other.m_UVs)
      , m_Indices(other.m_Indices)
    {
    }

    void Model::operator=(const Model& other)
    {
        m_Name = other.m_Name;
        m_VertexData = other.m_VertexData;
        m_Vertices = other.m_Vertices;
        m_Normals = other.m_Normals;
        m_UVs = other.m_UVs;
        m_Indices = other.m_Indices;

        if (m_VertexArray.id != 0)
        {
            DestroyArrays();
        }

        BuildArrays();
    }

	Model::Model(std::string path)
        : m_Name(std::string())
	{
		std::string fileType = std::string();

		size_t loc = path.find_last_of(".");
		if (loc != std::string::npos)
		{
			fileType = path.substr(loc + 1);
		}
		else
		{
			// warning
		}

		// probably a better way of doing this
		if (fileType.compare("obj") == 0)
		{
			ModelBuilder::Get().LoadOBJ(path, *this);
		}
		else if (fileType.compare("gltf") == 0)
		{
			ModelBuilder::Get().LoadGLTF(path, *this);
		}
	}

	void Model::BuildArrays()
	{
        m_VertexArray.Generate();
        m_VertexArray.Bind();

        m_VertexArray["Index"] = VertexBuffer(GL_ELEMENT_ARRAY_BUFFER);
        m_VertexArray["Index"].Generate();
        m_VertexArray["Index"].Bind();
        m_VertexArray["Index"].SetData<GLuint>(m_Indices.size(), m_Indices.data(), GL_STATIC_DRAW);

        m_VertexArray["Vertex"] = VertexBuffer(GL_ARRAY_BUFFER);
        m_VertexArray["Vertex"].Generate();
        m_VertexArray["Vertex"].Bind();
        m_VertexArray["Vertex"].SetData<glm::vec3>(m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW);
        m_VertexArray["Vertex"].SetAttPointer<GLfloat>(0, 3, GL_FLOAT, 3, 0);

        m_VertexArray["Vertex"].Unbind();

        if (m_Normals.size() > 0)
        {
            m_VertexArray["Normal"] = VertexBuffer(GL_ARRAY_BUFFER);
            m_VertexArray["Normal"].Generate();
            m_VertexArray["Normal"].Bind();
            m_VertexArray["Normal"].SetData<glm::vec3>(m_Normals.size(), m_Normals.data(), GL_STATIC_DRAW);
            m_VertexArray["Normal"].SetAttPointer<GLfloat>(1, 3, GL_FLOAT, 3, 0);
            m_VertexArray["Normal"].Unbind();
        }

        if (m_UVs.size() > 0)
        {
            m_VertexArray["UV"] = VertexBuffer(GL_ARRAY_BUFFER);
            m_VertexArray["UV"].Generate();
            m_VertexArray["UV"].Bind();
            m_VertexArray["UV"].SetData<glm::vec2>(m_UVs.size(), m_UVs.data(), GL_STATIC_DRAW);
            m_VertexArray["UV"].SetAttPointer<GLfloat>(2, 2, GL_FLOAT, 2, 0);
            m_VertexArray["UV"].Unbind();
        }

        m_VertexArray["EntityID"] = VertexBuffer(GL_ARRAY_BUFFER);
        m_VertexArray["EntityID"].Generate();
        m_VertexArray["EntityID"].Bind();
        m_VertexArray["EntityID"].SetData<GLfloat>(m_Vertices.size(), nullptr, GL_DYNAMIC_DRAW);
        m_VertexArray["EntityID"].SetAttPointer<GLfloat>(3, 1, GL_FLOAT, 1, 0);
        m_VertexArray["EntityID"].Unbind();

        m_VertexArray.Clear();
	}

    void Model::InitializeID(int id)
    {
        std::vector<int> data(m_Vertices.size(), id);

        m_VertexArray.Bind();
        m_VertexArray["EntityID"] = VertexBuffer(GL_ARRAY_BUFFER);
        m_VertexArray["EntityID"].Generate();
        m_VertexArray["EntityID"].Bind();
        m_VertexArray["EntityID"].SetData<GLint>(m_Vertices.size(), data.data(), GL_STATIC_DRAW);
        m_VertexArray["EntityID"].SetAttPointer<GLint>(3, 1, GL_INT, 1, 0);
        m_VertexArray["EntityID"].Unbind();
        m_VertexArray.Clear();
    }

    void Model::DestroyArrays()
    {
        m_VertexArray.Cleanup();
    }
}
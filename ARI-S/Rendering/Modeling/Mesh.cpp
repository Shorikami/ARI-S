#include <arpch.h>
#include "Mesh.h"


namespace ARIS
{
	Mesh::Mesh()
		: m_MeshName("Invalid")
		, m_VertexData(std::vector<Vertex>())
		, m_Indices(std::vector<unsigned int>())
		, m_Textures(std::vector<Texture>())
	{
	}

	Mesh::~Mesh()
	{
		DestroyArrays();
	}

	Mesh::Mesh(std::vector<Vertex> v, std::vector<unsigned int> i, std::vector<Texture> t, std::string name)
		: m_MeshName(name)
		, m_VertexData(v)
		, m_Indices(i)
		, m_Textures(t)
	{
		BuildArrays();
	}

	Mesh::Mesh(const Mesh& other)
	{
		m_MeshName = other.m_MeshName;
		m_VertexData = other.m_VertexData;
		m_Indices = other.m_Indices;
		m_Textures = other.m_Textures;

		BuildArrays();
	}

	void Mesh::operator=(const Mesh& other)
	{
		m_MeshName = other.m_MeshName;
		m_VertexData = other.m_VertexData;
		m_Indices = other.m_Indices;
		m_Textures = other.m_Textures;

		BuildArrays();
	}

	void Mesh::Draw(Shader& s, int entID)
	{
		unsigned diffNr = 1;
		unsigned specNr = 1;
		unsigned normNr = 1;
		unsigned heightNr = 1;
		unsigned metalNr = 1;
		unsigned roughNr = 1;
		unsigned metRoughNr = 1;

		// bool to check if metal + rough are a part of the same texture
		// (todo?: only would likely work with one texture; support for more?)
		bool combined = false;

		for (unsigned i = 0; i < m_Textures.size(); ++i)
		{
			std::string unit;
			glActiveTexture(GL_TEXTURE0 + i);

			switch (m_Textures[i].type)
			{
			case aiTextureType_DIFFUSE:
				unit = std::string("diffTex") + std::to_string(diffNr++);
				break;
			case aiTextureType_SPECULAR:
				unit = std::string("specTex") + std::to_string(specNr++);
				break;
			case aiTextureType_NORMALS:
				unit = std::string("normTex") + std::to_string(normNr++);
				break;
			case aiTextureType_HEIGHT:
				unit = std::string("heightTex") + std::to_string(heightNr++);
				break;
			case aiTextureType_METALNESS:
				unit = std::string("metalTex") + std::to_string(metalNr++);
				break;
			case aiTextureType_DIFFUSE_ROUGHNESS:
				unit = std::string("roughTex") + std::to_string(roughNr++);
				break;
			case aiTextureType_UNKNOWN:
				unit = std::string("metalRoughTex") + std::to_string(metRoughNr++);
				combined = true;
				break;
			}

			s.SetIntDirect(unit, i);
			m_Textures[i].Bind();
		}

		s.SetIntDirect("metRoughCombine", static_cast<int>(combined));

		m_VertexArray.Bind();

		std::vector<float> id(m_VertexData.size(), static_cast<float>(entID));

		m_VertexArray["EntityID"].Bind();
		m_VertexArray["EntityID"].UpdateData<GLfloat>(0, static_cast<GLuint>(m_VertexData.size()), id.data());
		m_VertexArray["EntityID"].Unbind();

		m_VertexArray.Draw(GL_TRIANGLES, static_cast<unsigned>(m_Indices.size()), GL_UNSIGNED_INT);
		m_VertexArray.Clear();

		glActiveTexture(0);
	}

	void Mesh::BuildArrays()
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
		m_VertexArray["Vertex"].SetData<Vertex>(m_VertexData.size(), m_VertexData.data(), GL_STATIC_DRAW);
		m_VertexArray["Vertex"].SetAttPointer<GLfloat>(0, 3, GL_FLOAT, sizeof(Vertex), 0, 0, true, true);
		m_VertexArray["Vertex"].SetAttPointer<GLfloat>(1, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, Vertex::s_Normal), 0, true, true);
		m_VertexArray["Vertex"].SetAttPointer<GLfloat>(2, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, Vertex::s_UV), 0, true, true);
		m_VertexArray["Vertex"].SetAttPointer<GLfloat>(3, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, Vertex::s_Tangent), 0, true, true);
		m_VertexArray["Vertex"].SetAttPointer<GLfloat>(4, 3, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, Vertex::s_Bitangent), 0, true, true);
		m_VertexArray["Vertex"].SetAttPointer<GLint>(5, 4, GL_INT, sizeof(Vertex), offsetof(Vertex, Vertex::m_BoneIDs), 0, true, true);
		m_VertexArray["Vertex"].SetAttPointer<GLfloat>(6, 4, GL_FLOAT, sizeof(Vertex), offsetof(Vertex, Vertex::m_BoneWeights), 0, true, true);

		m_VertexArray["Vertex"].Unbind();

		m_VertexArray["EntityID"] = VertexBuffer(GL_ARRAY_BUFFER);
		m_VertexArray["EntityID"].Generate();
		m_VertexArray["EntityID"].Bind();
		m_VertexArray["EntityID"].SetData<GLfloat>(m_VertexData.size(), nullptr, GL_DYNAMIC_DRAW);
		m_VertexArray["EntityID"].SetAttPointer<GLfloat>(3, 1, GL_FLOAT, 1, 0);
		m_VertexArray["EntityID"].Unbind();

		m_VertexArray.Clear();
	}

	void Mesh::DestroyArrays()
	{
		m_VertexArray.Cleanup();
	}
}
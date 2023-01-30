#include <hyspch.h>
#include "Model.h"

namespace Hayase
{
	Model::Model()
		: m_Mesh(nullptr)
		, m_InvertAxis(false)
	{
	}

	Model::Model(std::string path, glm::vec3 translation, glm::vec3 scale, float angle,
		RotationAxis axis, std::vector<std::pair<Texture*, std::string>> textures)
		: m_Translation(translation)
		, m_Scale(scale)
		, m_Angle(angle)
		, m_Axis(axis)
		, m_InvertAxis(false)
	{
		m_Mesh = new Mesh(path);
		m_Mesh->GenerateBuffers();

		m_Textures = textures;
	}

	Model::~Model()
	{
		if (m_Mesh)
		{
			m_Mesh->Cleanup();
			delete m_Mesh;
		}

		for (std::pair<Texture*, std::string> t : m_Textures)
		{
			delete t.first;
		}
	}

	void Model::Update()
	{
		glm::vec3 angleOfRot = glm::vec3(0.0f);

		switch (m_Axis)
		{
		case RotationAxis::xAxis:
			angleOfRot = glm::vec3(1.0f, 0.0f, 0.0f);
			break;
		case RotationAxis::yAxis:
			angleOfRot = glm::vec3(0.0f, 1.0f, 0.0f);
			break;
		case RotationAxis::zAxis:
			angleOfRot = glm::vec3(0.0f, 0.0f, 1.0f);
			break;
		}

		if (m_InvertAxis)
		{
			angleOfRot *= -1.0f;
		}

		m_Mesh->Update(m_Angle, m_Scale, m_Translation, angleOfRot);
	}
	
	void Model::Draw(GLuint shaderID, glm::mat4 view, glm::mat4 proj)
	{
		m_Mesh->Draw(shaderID, view, proj, m_Textures);
	}
}
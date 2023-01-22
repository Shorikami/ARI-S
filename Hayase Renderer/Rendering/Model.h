#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include "Texture.h"

#include <vector>
#include <string>

namespace Hayase
{
	class Model
	{
	public:
		enum class RotationAxis
		{
			xAxis = 0,
			yAxis = 1,
			zAxis = 2
		};

		Model();
		Model(std::string path, glm::vec3 translation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), float angle = 0.0f,
			RotationAxis axis = RotationAxis::yAxis,
			std::vector<std::pair<Texture*, std::string>> textures = std::vector<std::pair<Texture*, std::string>>());
		~Model();

		void Update();
		void Draw(GLuint shaderID, glm::mat4 view, glm::mat4 proj);

		std::string Name() { return m_Name; }
		void Name(std::string n) { m_Name = n; }

		glm::vec3& Translation() { return m_Translation; }
		float& Angle() { return m_Angle; }
		glm::vec3& Scale() { return m_Scale; }
		RotationAxis& Rotation() { return m_Axis; }
		bool& AxisInverted() { return m_InvertAxis; }

		Mesh* ModelMesh() { return m_Mesh; }
		std::vector<std::pair<Texture*, std::string>> Textures() { return m_Textures; }

		void Translate(glm::vec3 v) { m_Translation = v; }
		void Rotate(float angle) { m_Angle = angle; }
		void ScaleBy(glm::vec3 v) { m_Scale = v; }
		void RotateAround(RotationAxis axis) { m_Axis = axis; }
		void InvertAxis(bool b) { m_InvertAxis = b; }

		void ModelMesh(Mesh* m) { m_Mesh = m; }
		void Textures(std::vector<std::pair<Texture*, std::string>> t) { m_Textures = t; }

	private:
		Mesh* m_Mesh;
		std::vector<std::pair<Texture*, std::string>> m_Textures;

		glm::vec3 m_Translation;
		glm::vec3 m_Scale;

		float m_Angle;

		std::string m_Name;

		RotationAxis m_Axis;
		bool m_InvertAxis;
	};
}


#endif
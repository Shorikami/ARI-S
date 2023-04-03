#include <arpch.h>
#include "SceneSerializer.h"

#include "Entity.h"
#include "ModelBuilder.h"

#include <yaml-cpp/yaml.h>

namespace YAML 
{

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<ARIS::UUID>
	{
		static Node encode(const ARIS::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, ARIS::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

}

namespace ARIS
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity e)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << e.GetUUID();

		if (e.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tag = e.GetComponent<TagComponent>().s_Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap;
		}

		if (e.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& tc = e.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.GetTranslation();
			out << YAML::Key << "Rotation" << YAML::Value << tc.GetRotation();
			out << YAML::Key << "Scale" << YAML::Value << tc.GetScale();

			out << YAML::EndMap;
		}

		if (e.HasComponent<MeshComponent>())
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap;

			auto& mc = e.GetComponent<MeshComponent>();
			out << YAML::Key << "Name" << YAML::Value << mc.GetName();
			out << YAML::Key << "Path" << YAML::Value << mc.GetPath();
			out << YAML::Key << "Vertex Path" << YAML::Value << mc.GetVertexPath();
			out << YAML::Key << "Fragment Path" << YAML::Value << mc.GetFragmentPath();

			std::string texPath = std::string("N/A"), 
						normPath = std::string("N/A"),
						metPath = std::string("N/A"),
						roughPath = std::string("N/A"),
						metRoughPath = std::string("N/A");

			if (mc.GetDiffuseTex())
			{
				texPath = mc.GetDiffuseTex()->m_Path;
			}
			if (mc.GetNormalTex())
			{
				normPath = mc.GetNormalTex()->m_Path;
			}
			if (mc.GetMetallicTex())
			{
				metPath = mc.GetMetallicTex()->m_Path;
			}
			if (mc.GetRoughnessTex())
			{
				roughPath = mc.GetRoughnessTex()->m_Path;
			}
			if (mc.GetMetalRough())
			{
				metRoughPath = mc.GetMetalRough()->m_Path;
			}

			out << YAML::Key << "Diffuse Path" << YAML::Value << texPath;
			out << YAML::Key << "Normal Path" << YAML::Value << normPath;
			out << YAML::Key << "Metallic Path" << YAML::Value << metPath;
			out << YAML::Key << "Roughness Path" << YAML::Value << roughPath;
			out << YAML::Key << "Metal/Roughness Path" << YAML::Value << metRoughPath;

			out << YAML::EndMap;
		}

		if (e.HasComponent<PointLightComponent>())
		{
			out << YAML::Key << "PointLightComponent";
			out << YAML::BeginMap;

			auto& lc = e.GetComponent<PointLightComponent>();
			//out << YAML::Key << "Vertex Path" << YAML::Value << lc.GetVertexPath();
			//out << YAML::Key << "Fragment Path" << YAML::Value << lc.GetFragmentPath();
			out << YAML::Key << "Color" << YAML::Value << lc.GetColor();
			out << YAML::Key << "Range" << YAML::Value << lc.GetRange();
			out << YAML::Key << "Intensity" << YAML::Value << lc.GetIntensity();

			out << YAML::EndMap;
		}

		if (e.HasComponent<DirectionLightComponent>())
		{
			out << YAML::Key << "DirectionLightComponent";
			out << YAML::BeginMap;

			auto& lc = e.GetComponent<DirectionLightComponent>();
			//out << YAML::Key << "Vertex Path" << YAML::Value << lc.GetVertexPath();
			//out << YAML::Key << "Fragment Path" << YAML::Value << lc.GetFragmentPath();
			out << YAML::Key << "Color" << YAML::Value << lc.GetColor();
			out << YAML::Key << "Width" << YAML::Value << lc.GetWidth();
			out << YAML::Key << "Height" << YAML::Value << lc.GetHeight();
			out << YAML::Key << "Near" << YAML::Value << lc.GetNear();
			out << YAML::Key << "Far" << YAML::Value << lc.GetFar();

			out << YAML::EndMap;
		}

		out << YAML::EndMap; // Entity
	}
	
	SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene)
		: m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Test";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.each([&](auto id)
		{
			Entity e = { id, m_Scene.get() };
			if (!e)
			{
				return;
			}

			SerializeEntity(out, e);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{

	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
		{
			return false;
		}

		std::string sceneName = data["Scene"].as<std::string>();

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto e : entities)
			{
				uint64_t uuid = e["Entity"].as<uint64_t>();

				std::string name;
				auto tag = e["TagComponent"];
				if (tag)
				{
					name = tag["Tag"].as<std::string>();
				}

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto tc = e["TransformComponent"];
				if (tc)
				{
					auto& t = deserializedEntity.GetComponent<TransformComponent>();
					t.m_Translation = tc["Translation"].as<glm::vec3>();
					t.m_Rotation = tc["Rotation"].as<glm::vec3>();
					t.m_Scale = tc["Scale"].as<glm::vec3>();
				}

				auto mc = e["MeshComponent"];
				if (mc)
				{
					auto& t = deserializedEntity.AddComponent<MeshComponent>();
					std::string name = mc["Name"].as<std::string>();
					std::string path = mc["Path"].as<std::string>();
					std::string vSrc = mc["Vertex Path"].as<std::string>();
					std::string fSrc = mc["Fragment Path"].as<std::string>();

					std::string diffTex = mc["Diffuse Path"].as<std::string>();
					std::string normTex = mc["Normal Path"].as<std::string>();
					std::string metTex = mc["Metallic Path"].as<std::string>();
					std::string roughTex = mc["Roughness Path"].as<std::string>();
					std::string metalRoughTex = mc["Metal/Roughness Path"].as<std::string>();

					t.SetName(name);
					t.SetPath(path);
					ModelBuilder::Get().LoadModel(path, t.m_Model);

					if (diffTex != "N/A")
					{
						t.m_DiffuseTex = new Texture(diffTex, GL_LINEAR, GL_REPEAT, false, aiTextureType_DIFFUSE);
						
						for (Mesh& m : t.m_Model.m_Meshes)
						{
							m.m_Textures.push_back(*t.m_DiffuseTex);
						}
					}
					//else
					//{
					//	std::vector<Texture> textures = t.m_Model.GetLoadedTextures();
					//
					//	auto tt = std::find_if(std::begin(textures), std::end(textures),
					//		[&](Texture const& tx) { return tx.type == aiTextureType_DIFFUSE; });
					//
					//	if (tt != std::end(textures))
					//	{
					//		t.m_DiffuseTex = new Texture((*tt).m_Path, GL_LINEAR, GL_REPEAT, false, aiTextureType_DIFFUSE);
					//	}
					//}

					if (normTex != "N/A")
					{
						t.m_NormalTex = new Texture(normTex, GL_LINEAR, GL_REPEAT, false, aiTextureType_NORMALS);

						for (Mesh& m : t.m_Model.m_Meshes)
						{
							m.m_Textures.push_back(*t.m_NormalTex);
						}
					}
					//else
					//{
					//	std::vector<Texture> textures = t.m_Model.GetLoadedTextures();
					//
					//	auto tt = std::find_if(std::begin(textures), std::end(textures),
					//		[&](Texture const& tx) { return tx.type == aiTextureType_NORMALS; });
					//
					//	if (tt != std::end(textures))
					//	{
					//		t.m_NormalTex = new Texture((*tt).m_Path, GL_LINEAR, GL_REPEAT, false, aiTextureType_NORMALS);
					//	}
					//}

					if (metTex != "N/A")
					{
						t.m_Metallic = new Texture(metTex, GL_LINEAR, GL_REPEAT, false, aiTextureType_METALNESS);

						for (Mesh& m : t.m_Model.m_Meshes)
						{
							m.m_Textures.push_back(*t.m_Metallic);
						}
					}
					//else
					//{
					//	std::vector<Texture> textures = t.m_Model.GetLoadedTextures();
					//
					//	auto tt = std::find_if(std::begin(textures), std::end(textures),
					//		[&](Texture const& tx) { return tx.type == aiTextureType_METALNESS; });
					//
					//	if (tt != std::end(textures))
					//	{
					//		t.m_Metallic = new Texture((*tt).m_Path, GL_LINEAR, GL_REPEAT, false, aiTextureType_METALNESS);
					//	}
					//}

					if (roughTex != "N/A")
					{
						t.m_Roughness = new Texture(roughTex, GL_LINEAR, GL_REPEAT, false, aiTextureType_DIFFUSE_ROUGHNESS);

						for (Mesh& m : t.m_Model.m_Meshes)
						{
							m.m_Textures.push_back(*t.m_Roughness);
						}
					}
					//else
					//{
					//	std::vector<Texture> textures = t.m_Model.GetLoadedTextures();
					//
					//	auto tt = std::find_if(std::begin(textures), std::end(textures),
					//		[&](Texture const& tx) { return tx.type == aiTextureType_DIFFUSE_ROUGHNESS; });
					//
					//	if (tt != std::end(textures))
					//	{
					//		t.m_Roughness = new Texture((*tt).m_Path, GL_LINEAR, GL_REPEAT, false, aiTextureType_DIFFUSE_ROUGHNESS);
					//	}
					//}

					if (metalRoughTex != "N/A")
					{
						t.m_MetalRough = new Texture(metalRoughTex, GL_LINEAR, GL_REPEAT, false, aiTextureType_UNKNOWN);

						for (Mesh& m : t.m_Model.m_Meshes)
						{
							m.m_Textures.push_back(Texture(metalRoughTex, GL_LINEAR, GL_REPEAT, false, aiTextureType_UNKNOWN));
						}
					}
					//else
					//{
					//	std::vector<Texture> textures = t.m_Model.GetLoadedTextures();
					//
					//	auto tt = std::find_if(std::begin(textures), std::end(textures),
					//		[&](Texture const& tx) { return tx.type == aiTextureType_UNKNOWN; });
					//
					//	if (tt != std::end(textures))
					//	{
					//		t.m_MetalRough = new Texture((*tt).m_Path, GL_LINEAR, GL_REPEAT, false, aiTextureType_UNKNOWN);
					//	}
					//}

					t.m_VertexSrc = vSrc;
					t.m_FragmentSrc = fSrc;
					t.ReloadShader();
				}

				auto lc = e["PointLightComponent"];
				if (lc)
				{
					auto& t = deserializedEntity.AddComponent<PointLightComponent>();
					//std::string vSrc = lc["Vertex Path"].as<std::string>();
					//std::string fSrc = lc["Fragment Path"].as<std::string>();

					//t.m_VertexSrc = vSrc;
					//t.m_FragmentSrc = fSrc;
					t.m_Color = lc["Color"].as<glm::vec4>();
					t.m_Range = lc["Range"].as<float>();
					t.m_Intensity = lc["Intensity"].as<float>();

					//t.ReloadShader();
				}

				auto dl = e["DirectionLightComponent"];
				if (dl)
				{
					auto& t = deserializedEntity.AddComponent<DirectionLightComponent>();
					//std::string vSrc = lc["Vertex Path"].as<std::string>();
					//std::string fSrc = lc["Fragment Path"].as<std::string>();

					//t.m_VertexSrc = vSrc;
					//t.m_FragmentSrc = fSrc;
					t.m_Color = dl["Color"].as<glm::vec4>();
					t.m_Width = dl["Width"].as<float>();
					t.m_Height = dl["Height"].as<float>();
					t.m_Near = dl["Near"].as<float>();
					t.m_Far = dl["Far"].as<float>();

					//t.ReloadShader();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		return false;
	}
}
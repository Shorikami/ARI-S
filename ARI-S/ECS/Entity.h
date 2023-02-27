#include "Transform.hpp"
#include "Mesh.hpp"
#include "PointLight.hpp"
#include "DirectionalLight.hpp"

#include "UUID.hpp"

#include "entt.hpp"

namespace ARIS
{
	class Scene;

	struct IDComponent
	{
		UUID s_ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string s_Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: s_Tag(tag)
		{
		}
	};

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template <typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			T& comp = m_SceneContext->m_Registry.emplace<T>(m_Handle, std::forward<Args>(args)...);
			return comp;
		}

		template <typename T>
		void RemoveComponent()
		{
			m_SceneContext->m_Registry.remove<T>(m_Handle);
		}

		template <typename T>
		T& GetComponent()
		{
			return m_SceneContext->m_Registry.get<T>(m_Handle);
		}

		template <typename T>
		bool HasComponent()
		{
			return m_SceneContext->m_Registry.any_of<T>(m_Handle);
		}

		operator bool() const { return m_Handle != entt::null; }
		operator entt::entity() const { return m_Handle; }
		operator uint32_t() const { return (uint32_t)m_Handle; }

		UUID GetUUID() { return GetComponent<IDComponent>().s_ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().s_Tag; }

		bool operator==(const Entity& other) const
		{
			return m_Handle == other.m_Handle && m_SceneContext == other.m_SceneContext;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

	private:
		entt::entity m_Handle{ entt::null };
		Scene* m_SceneContext = nullptr;
	};
}
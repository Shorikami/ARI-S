#include <hyspch.h>

#include "Entity.h"

namespace Hayase
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Handle(handle)
		, m_SceneContext(scene)
	{
	}

	template <typename T, typename... Args>
	T& Entity::AddComponent(Args&&... args)
	{
		T& comp = m_SceneContext->m_Registry.emplace<T>(m_Handle, std::forward<Args>(args));
		return comp;
	}

	template <typename T>
	void Entity::RemoveComponent()
	{
		m_SceneContext->m_Registry.remove<T>(m_Handle);
	}

	template <typename T>
	T& Entity::GetComponent()
	{
		return m_SceneContext->m_Registry.get<T>(m_Handle);
	}

	template <typename T>
	bool Entity::HasComponent()
	{
		return m_SceneContext->m_Registry.any_of<T>(m_Handle);
	}
}
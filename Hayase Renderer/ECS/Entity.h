#include "entt.hpp"

#ifndef ENTITY_H
#define ENTITY_H

#include "Transform.hpp"
#include "SceneFramework.h"
#include "UUID.hpp"

namespace Hayase
{
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
		T& AddComponent(Args&&... args);

		template <typename T>
		void RemoveComponent();

		template <typename T>
		T& GetComponent();

		template <typename T>
		bool HasComponent();

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

#endif
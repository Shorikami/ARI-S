#include <hyspch.h>

#include "Entity.h"
#include "SceneFramework.h"

namespace Hayase
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Handle(handle)
		, m_SceneContext(scene)
	{
	}
}
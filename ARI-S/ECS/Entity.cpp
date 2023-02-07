#include <arpch.h>

#include "Entity.h"
#include "Scene.h"

namespace ARIS
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Handle(handle)
		, m_SceneContext(scene)
	{
	}
}
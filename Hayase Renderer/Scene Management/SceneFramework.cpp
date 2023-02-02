#include <hyspch.h>

#include "SceneFramework.h"
#include "Entity.h"

namespace Hayase
{
    Scene::Scene() 
        : _windowWidth(100)
        , _windowHeight(100)
    {

    }

    Scene::Scene(int windowWidth, int windowHeight)
        : _windowWidth(windowWidth)
        , _windowHeight(windowHeight)
    {
    }

    Scene::~Scene()
    {
        CleanUp();
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }
    
    Entity Scene::CreateEntityWithUUID(UUID id, const std::string& name)
    {
        Entity e = { m_Registry.create(), this };
        e.AddComponent<IDComponent>(id);
        e.AddComponent<TransformComponent>(); // an entity must always have a transform
        auto& tag = e.AddComponent<TagComponent>();
        tag.s_Tag = name.empty() ? "Entity" : name;
        
        m_EntityMap[id] = e;
        
        return e;
    }
    
    void Scene::DestroyEntity(Entity e)
    {
        m_EntityMap.erase(e.GetUUID());
        m_Registry.destroy(e);
    }
    
    Entity Scene::FindEntityByName(std::string_view name)
    {
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view)
        {
            const TagComponent& tc = view.get<TagComponent>(entity);
            if (tc.s_Tag == name)
            {
                return Entity{ entity, this };
            }
        }
        return {};
    }
    
    Entity Scene::FindEntityByUUID(UUID uuid)
    {
        if (m_EntityMap.find(uuid) != m_EntityMap.end())
        {
            return { m_EntityMap.at(uuid), this };
        }
        return {};
    }

    int Scene::Init()
    {
        return -1;
    }

    int Scene::PreRender()
    {
        return -1;
    }

    int Scene::Render()
    {
        return -1;
    }

    int Scene::PostRender()
    {
        return -1;
    }

    void Scene::OnViewportResize(uint32_t w, uint32_t h)
    {
        return;
    }

    void Scene::CleanUp()
    {
        return;
    }

    int Scene::Display()
    {
        PreRender();
        Render();
        PostRender();

        return -1;
    }
}
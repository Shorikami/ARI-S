#include <hyspch.h>

#include "Scene.h"
#include "Entity.h"

namespace Hayase
{
    Scene::Scene() 
        : _windowWidth(100)
        , _windowHeight(100)
    {
        Init();
    }

    Scene::Scene(int windowWidth, int windowHeight)
        : _windowWidth(windowWidth)
        , _windowHeight(windowHeight)
    {
        Init();
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
        // Scene FBO (for the editor)
        m_SceneFBO = new Framebuffer(_windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_SceneFBO->Bind();
        m_SceneFBO->AllocateAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA, GL_UNSIGNED_BYTE);
        m_SceneFBO->DrawBuffers();
        m_SceneFBO->AllocateAttachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_FLOAT);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Uh oh! Scene FBO is incomplete!" << std::endl;
            m_SceneFBO->Unbind();
            return -1;
        }

        m_SceneFBO->Unbind();

        return 0;
    }

    int Scene::PreRender()
    {
        return 0;
    }

    int Scene::Render()
    {
        glClearColor(0.1f, 1.0f, 0.5f, 1.0f);
        m_SceneFBO->Activate();
        m_SceneFBO->Unbind();

        return 0;
    }

    int Scene::PostRender()
    {
        return 0;
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

    void Scene::Update(DeltaTime dt)
    {
        m_DT = dt.GetSeconds();

        Display();
    }

    void Scene::OnImGuiRender()
    {
    
    }

    void Scene::OnEvent(Event& e)
    {
        std::cout << e.ToString() << std::endl;
    }
}
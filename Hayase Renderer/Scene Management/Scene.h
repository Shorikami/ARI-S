#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window.h"
#include "FrameMemory.hpp"
#include "Timer.h"
#include "Camera.h"
#include "UUID.hpp"
#include "Layer.h"

#include "entt.hpp"

namespace Hayase
{
    class Entity;

    class Scene : public Layer
    {

    public:
        Scene();
        Scene(int windowWidth, int windowHeight);
        virtual ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID id, const std::string& name = std::string());
        void DestroyEntity(Entity e);
        
        Entity FindEntityByName(std::string_view name);
        Entity FindEntityByUUID(UUID uuid);

        virtual int Init();
        virtual int Display();

        virtual void Update(DeltaTime dt);

        virtual void CleanUp();

        virtual int PreRender();
        virtual int Render();
        virtual int PostRender();

        virtual void OnViewportResize(uint32_t w, uint32_t h);

        Framebuffer* GetSceneFBO() { return m_SceneFBO; }
        Camera& GetCamera() { return m_Camera; }
    public:
        void OnImGuiRender() override;
        void OnEvent(Event& e) override;

    protected:
        int _windowHeight, _windowWidth;

        Framebuffer* m_SceneFBO;
        DeltaTime m_DT;
        Camera m_Camera;

        entt::registry m_Registry;
        
        std::unordered_map<UUID, entt::entity> m_EntityMap;

        friend class Entity;
        friend class HierarchyPanel;
        friend class SceneSerializer;
    };
}

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window.h"
#include "FrameMemory.hpp"
#include "Timer.h"
#include "Camera.h"
#include "UUID.hpp"
#include "Layer.h"

#include "Texture.h"
#include "Shader.h"
#include "UniformMemory.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <vector>

#include "entt.hpp"

namespace ARIS
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

    // TODO: UPDATE THIS TO THE NEW ENGINE SYSTEM
    // this is temporary in case I don't rework everything before I start
    // working on moment shadow maps
    private:
        void InitMembers();
        void ReloadShaders();

        void GenerateLocalLights();

        void RenderLocalLights();
        void RenderSkybox();

        GLuint quadVAO, quadVBO;
        void RenderQuad();

        LocalLight localLights[MAX_LIGHTS];

        UniformBuffer<World>* matrixData;
        UniformBuffer<Lights>* lightData;
        UniformBuffer<LocalLight>* localLightData;

        //std::vector<Model*> models;
        //Model *cube, *sphere, *skybox;

        Shader* geometryPass, * lightingPass, * localLight, * flatShader;
        Shader* skyboxShader;

        std::vector<Texture*> gTextures;

        std::vector<std::pair<Texture, std::string>> textures, groundTextures, skyboxTextures;

        Framebuffer* gBuffer;

        // ImGUI options
        glm::vec3 m_BGColor = glm::vec3(51.0f / 255.0f, 102.0f / 255.0f, 140.0f / 255.0f);
        glm::vec3 m_LightColor = glm::vec3(0.7f);

        float minX = -4.0f, maxX = 10.0f, minY = -1.f, maxY = 15.f, minZ = -4.0f, maxZ = 14.0f, minRange = 1.0f, maxRange = 10.0f;

        bool m_DisplayDebugRanges = false, m_DisplayLightPassLocations = true,
            m_DisplayLocalLights = true, m_DisplaySkybox = true;

        int m_RenderOption = 0, m_SelectedModelIdx = 0;
    };
}

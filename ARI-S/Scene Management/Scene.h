#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window.h"
#include "FrameMemory.hpp"
#include "Timer.h"
#include "Cameras/EditorCamera.h"
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

        int Init();

        void UpdateEditor(DeltaTime dt, EditorCamera& edCam);
        //void UpdateRuntime(DeltaTime dt);

        void CleanUp();

        int PreRender();

        int RenderEditor(EditorCamera& editorCam);
        //int RenderRuntime();

        int PostRender();

        virtual void OnViewportResize(uint32_t w, uint32_t h);

        Framebuffer*& GetSceneFBO() { return m_SceneFBO; }
        Framebuffer*& GetGBuffer() { return gBuffer; }

    public:
        void OnImGuiRender() override;
        void OnEvent(Event& e) override;

    protected:
        int _windowHeight, _windowWidth;

        Framebuffer* m_SceneFBO;
        DeltaTime m_DT;

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

        void GenerateBasicShapes();

        void GenerateIBL();
        void GenerateSphereHarmonics();

        GLuint cubeVAO, cubeVBO;
        void RenderSkybox(glm::mat4 view, glm::mat4 proj);
        void RenderHDRMap(glm::mat4 view, glm::mat4 proj);

        GLuint quadVAO, quadVBO;
        void RenderQuad();

        LocalLight localLights[MAX_LIGHTS];

        UniformBuffer<World>* matrixData;
        UniformBuffer<BlurKernel>* kernelData;
        UniformBuffer<Discrepancy>* hammersleyData;
        UniformBuffer<HarmonicColors>* harmonicData;

        Shader* geometryPass, * lightingPass, * localLight, * flatShader;
        
        Shader* shadowPass, *computeBlur;
        Shader* aoPass, * aoBlur;

        Shader* skyboxShader;

        // PBS + IBL
        Shader * hdrMapping, * hdrEnvironment, * irradiance,  * mapFilter, * brdf;

        // Deferred
        std::vector<Texture*> gTextures;

        // Shadows
        Texture* sDepthMap, * blurOutput;

        // AO
        Texture* aoMap, * aoBlurOutput;

        // Skybox
        Texture* skybox;

        // PBS + IBL
        Texture* hdrTexture, * hdrCubemap, *filteredHDR, *irradianceTex, *brdfTex;
        Texture* outputIrradiance, *outputIrrTex;

        // G-Buffer
        Framebuffer* gBuffer;

        // Shadow blur buffer
        Framebuffer* sBuffer;

        // AO buffer
        Framebuffer* aoBuffer;

        // Environment map buffer
        Framebuffer* captureBuffer;
    };
}

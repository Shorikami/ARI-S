#ifndef DEFERRED_H
#define DEFERRED_H

#include "SceneFramework.h"
#include "Texture.h"
#include "Shader.h"
#include "Layer.h"
#include "Memory/UniformMemory.hpp"

#include "Model.h"
#include "OBJReader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <vector>

namespace Hayase
{
    class Deferred : public Scene, public Layer
    {

    public:
        Deferred() = default;
        Deferred(int windowWidth, int windowHeight);
        virtual ~Deferred();

    public:
        void OnAttach() override;
        void OnDetach() override;
        void Update(DeltaTime dt) override;
        void OnImGuiRender() override;
        void OnEvent(Event& e) override;

    public:
        int Init() override;
        void CleanUp() override;

        int PreRender() override;
        int Render() override;
        int PostRender() override;

        void OnViewportResize(uint32_t w, uint32_t h) override;

    private:
        void initMembers();
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

        std::vector<Model*> models;
        Mesh *cube, *sphere, *skybox;
        OBJReader reader;

        Shader *geometryPass, *lightingPass, *localLight, *flatShader;
        Shader *skyboxShader;

        std::vector<Texture*> gTextures;

        std::vector<std::pair<Texture*, std::string>> textures, groundTextures, skyboxTextures;

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

#endif

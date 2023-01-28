#ifndef DEFERRED_H
#define DEFERRED_H

#include "SceneFramework.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "Shader.h"
#include "Camera.h"
#include "../../Rendering/Memory/UniformMemory.hpp"

#include "../../Rendering/Model.h"
#include "../../Rendering/OBJReader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <vector>

namespace Hayase
{
    class Deferred : public Scene
    {

    public:
        Deferred() = default;
        Deferred(int windowWidth, int windowHeight);
        virtual ~Deferred();


    public:
        int Init() override;
        void CleanUp() override;

        int PreRender(float frame) override;
        int Render() override;
        int PostRender() override;

        void ProcessInput(GLFWwindow* w, float dt) override;
    private:
        void initMembers();
        void ReloadShaders();

        void GenerateLocalLights();

        void RenderLocalLights();
        void RenderSkybox();

        GLuint quadVAO, quadVBO;
        void RenderQuad();

        Camera m_Camera;

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

        GLfloat   angleOfRotation;

        // ImGUI options
        glm::vec3 m_BGColor = glm::vec3(51.0f / 255.0f, 102.0f / 255.0f, 140.0f / 255.0f);
        glm::vec3 m_LightColor = glm::vec3(0.7f);

        float minX = -4.0f, maxX = 10.0f, minY = -1.f, maxY = 15.f, minZ = -4.0f, maxZ = 14.0f, minRange = 1.0f, maxRange = 10.0f;

        bool m_DisplayDebugRanges = false, m_DisplayLightPassLocations = true,
            m_DisplayLocalLights = true, m_DisplaySkybox = true, m_EditorMode = false;
        
        int m_RenderOption = 0,  m_SelectedModelIdx = 0;
    };
}

#endif

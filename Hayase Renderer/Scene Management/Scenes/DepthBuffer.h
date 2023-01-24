#ifndef DEPTHBUFFER_H
#define DEPTHBUFFER_H

#include "../SceneFramework.h"
#include "../../Rendering/Texture.h"
#include "../../Rendering/Framebuffer.h"
#include "../../Rendering/Shader.h"
#include "../../Rendering/Camera.h"
#include "../../Rendering/Memory/UniformMemory.hpp"

#include "../../Rendering/Model.h"
#include "../../Rendering/OBJReader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

namespace Hayase
{
    class DepthBuffer : public Scene
    {

    public:
        DepthBuffer() = default;
        DepthBuffer(int windowWidth, int windowHeight);
        virtual ~DepthBuffer();


    public:
        int Init() override;
        void CleanUp() override;

        int PreRender(float frame) override;
        int Render() override;
        int PostRender() override;

        void ProcessInput(GLFWwindow* w, float dt) override;

    private:
        void initMembers();

        void RenderSkybox();

        GLuint quadVAO, quadVBO;
        void RenderQuad();

        Camera m_Camera;

        UniformBuffer<World>* matrixData;

        Model* loadedObj[4], * quadPlane[3];
        Mesh* skybox;
        OBJReader reader;

        Shader* depthPass, *FSQShader;
        Shader* skyboxShader;

        Texture* depthTexture;

        std::vector<std::pair<Texture*, std::string>> skyboxTextures;

        Framebuffer* depthBuffer;

        unsigned int depthMapFBO, depthMap;

        glm::vec3 m_BGColor = glm::vec3(51.0f / 255.0f, 102.0f / 255.0f, 140.0f / 255.0f);
    };
}

#endif

#ifndef DEFAULT_H
#define DEFAULT_H

#include "../SceneFramework.h"
#include "../../Rendering/Texture.h"
#include "../../Rendering/Framebuffer.h"
#include "../../Rendering/Shader.h"
#include "../../Rendering/Camera.h"
#include "../../Rendering/Memory/UniformMemory.hpp"

#include "../../Rendering/Mesh.h"
#include "../../Rendering/OBJReader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

namespace Hayase
{
    class Default : public Scene
    {

    public:
        Default() = default;
        Default(int windowWidth, int windowHeight);
        virtual ~Default();


    public:
        int Init() override;
        void CleanUp() override;

        int preRender() override;
        int Render() override;
        int postRender() override;

        void ProcessInput(GLFWwindow* w, float dt) override;
        void ProcessMouse(float x, float y) override;


    private:
        void initMembers();

        void RenderActualScene();
        void RenderLights(glm::mat4 view = glm::mat4(1.0f));
        void RenderSkybox(glm::mat4 view = glm::mat4(1.0f));

        GLuint quadVAO, quadVBO;
        void RenderQuad();

        Camera m_Camera;

        UniformBuffer<World>* matrixData;
        UniformBuffer<Lights>* lightData;

        Mesh* loadedObj, * sphere[16], * sphereLine, * quadPlane, * skybox;
        OBJReader reader;


        Shader *lineShader, *sphereShader[16];
        Shader *geometryPass, *lightingPass, *localLight;

        Shader *skyboxShader;

        std::vector<Texture*> gTextures;
        std::vector<std::pair<Texture*, std::string>> textures, skyboxTextures;

        Framebuffer* gBuffer;

        // Framebuffer Info
        GLuint gBuf;
        GLuint gTex[6]; // gPosition, gNormal, gUVs, gAlbedo, gSpecular, gDepth
        GLuint rbo;

        GLfloat   angleOfRotation;

        // ImGUI options
        glm::vec3 m_BGColor = glm::vec3(51.0f / 255.0f, 102.0f / 255.0f, 140.0f / 255.0f);
        glm::vec3 m_LightColor = glm::vec3(0.7f);

        bool m_DisplayFaceNorms = false, m_DisplayVertNorms = false;
    };
}

#endif

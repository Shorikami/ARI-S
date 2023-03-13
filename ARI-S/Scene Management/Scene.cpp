#include <arpch.h>

#include "Scene.h"
#include "Entity.h"

#include <vec3.hpp>
#include <glad/glad.h>

#include <gtc/matrix_access.hpp>
#include <gtc/type_ptr.hpp>

#include "imgui.h"

#include "Math/Vector.h"
#include "Math/Cholesky.hpp"
#include "SphereHarmonics.hpp"

//#include "stb_image.h"

unsigned currLights = 4;
int currLocalLights = NUM_LIGHTS;

int kernelSize = 10;
int gaussianWeight = 10;

namespace ARIS
{
    float RandomNum(float min, float max)
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }

    float Gaussian(float i, float s)
    {
        float mult = powf(i / s, 2.0f);
        return std::exp(-0.5f * mult);
    }

#pragma region Entity
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
#pragma endregion Entity

    Scene::Scene() 
        : _windowWidth(100)
        , _windowHeight(100)
    {
        InitMembers();

        Init();
    }

    Scene::Scene(int windowWidth, int windowHeight)
        : _windowWidth(windowWidth)
        , _windowHeight(windowHeight)
    {
        InitMembers();

        Init();
    }

    void Scene::InitMembers()
    {
        matrixData = new UniformBuffer<World>(0);
        kernelData = new UniformBuffer<BlurKernel>(3);
        hammersleyData = new UniformBuffer<Discrepancy>(4);
        harmonicData = new UniformBuffer<HarmonicColors>(5);
    }

    void Scene::ReloadShaders()
    {
        delete lightingPass;
        delete shadowPass;
        delete computeBlur;
        
        lightingPass = new Shader(false, "Deferred/LightingPassShadows.vert", "Deferred/LightingPassShadows.frag");
        shadowPass = new Shader(false, "Shadows/Moment/Shadows.vert", "Shadows/Moment/Shadows.frag");
        computeBlur = new Shader(false, "Shadows/ConvolutionBlur.cmpt");
    }

    void Scene::GenerateBasicShapes()
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    Scene::~Scene()
    {
        CleanUp();
    }

    int Scene::Init()
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        skyboxShader = new Shader(false, "Skybox.vert", "Skybox.frag");
        hdrMapping = new Shader(false, "IBL/CubemapHDR.vert", "IBL/CubemapHDR.frag");
        hdrEnvironment = new Shader(false, "IBL/Environment.vert", "IBL/Environment.frag");
        //irrComp = new Shader(false, "IBL/IrradianceConvolution.cmpt");

        geometryPass = new Shader(false, "Deferred/GeometryPass.vert", "Deferred/GeometryPass.frag");
        lightingPass = new Shader(false, "Deferred/LightingPassShadows.vert", "Deferred/LightingPassShadows.frag");
        localLight = new Shader(false, "Deferred/LocalLight.vert", "Deferred/LocalLight.frag");

        shadowPass = new Shader(false, "Shadows/Moment/Shadows.vert", "Shadows/Moment/Shadows.frag");
        computeBlur = new Shader(false, "Shadows/ConvolutionBlur.cmpt");

        hdrTexture = new Texture("Content/Assets/Textures/HDR/Winter_Forest.hdr", GL_LINEAR, GL_CLAMP_TO_EDGE, true);
        hdrCubemap = new Texture();
        hdrCubemap->AllocateCubemap(512, 512, GL_RGBA16F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_FLOAT);

        irrBlurOutput = new Texture();
        irrBlurOutput->AllocateCubemap(512, 512, GL_RGBA32F, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_FLOAT);

        // gBuffer textures (position, normals, UVs, albedo (diffuse), specular, depth)
        for (unsigned i = 0; i < 5; ++i)
        {
            gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_RGBA16F, GL_RGBA, nullptr,
                GL_NEAREST, GL_CLAMP_TO_EDGE));
        }
        // Single-channel red texture (for entity ID and mouse-clicking; this will be
        // shared with the scene FBO)
        gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_R32F, GL_RED, nullptr,
            GL_NEAREST, GL_CLAMP_TO_BORDER, GL_UNSIGNED_BYTE));

        gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, nullptr,
            GL_NEAREST, GL_REPEAT, GL_FLOAT));

        sDepthMap = new Texture(2048, 2048, GL_RGBA32F, GL_RGBA, nullptr,
            GL_NEAREST, GL_CLAMP_TO_BORDER, GL_UNSIGNED_BYTE);

        blurOutput = new Texture(2048, 2048, GL_RGBA32F, GL_RGBA, nullptr,
            GL_NEAREST, GL_CLAMP_TO_BORDER, GL_FLOAT);

        captureBuffer = new Framebuffer(true);
        captureBuffer->Bind(true);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureBuffer->GetRBO());
        captureBuffer->Unbind(true);

        // gBuffer FBO
        gBuffer = new Framebuffer(_windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gBuffer->Bind();
        for (unsigned i = 0; i < gTextures.size() - 2; ++i)
        {
            gBuffer->AttachTexture(GL_COLOR_ATTACHMENT0 + i, *gTextures[i]);
        }
        gBuffer->AttachTexture(GL_COLOR_ATTACHMENT5, *gTextures[gTextures.size() - 2]);
        gBuffer->DrawBuffers();
        
        gBuffer->AttachTexture(GL_DEPTH_ATTACHMENT, *gTextures[gTextures.size() - 1]);

        gBuffer->Unbind();

        // Generate quad + skybox cube
        GenerateBasicShapes();

        // Scene FBO (for the editor)
        m_SceneFBO = new Framebuffer(_windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_SceneFBO->Bind();
        m_SceneFBO->AllocateAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA32F, GL_RGBA, GL_UNSIGNED_BYTE);
        m_SceneFBO->AllocateAttachTexture(GL_COLOR_ATTACHMENT1, GL_R32F, GL_RED, GL_UNSIGNED_BYTE);
        //m_SceneFBO->AttachTexture(GL_COLOR_ATTACHMENT1, *gTextures[gTextures.size() - 2]);
        m_SceneFBO->DrawBuffers();
        m_SceneFBO->AllocateAttachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Uh oh! Scene FBO is incomplete!" << std::endl;
            m_SceneFBO->Unbind();
            return -1;
        }

        m_SceneFBO->Unbind();

        // Shadow FBO 
        sBuffer = new Framebuffer(2048, 2048, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sBuffer->Bind();

        sBuffer->AttachTexture(GL_COLOR_ATTACHMENT0, *sDepthMap);
        sBuffer->DrawBuffers();
        sBuffer->AllocateAttachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Uh oh! Shadow FBO is incomplete!" << std::endl;
            sBuffer->Unbind();
            return -1;
        }

        sBuffer->Unbind();

        glm::mat4 hdrProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 hdrView[] =
        {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        hdrMapping->Activate();
        hdrMapping->SetInt("hdrMap", 0);
        hdrMapping->SetMat4("projection", hdrProj);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture->m_ID);

        glViewport(0, 0, 512, 512);
        captureBuffer->Bind();

        for (unsigned i = 0; i < 6; ++i)
        {
            hdrMapping->SetMat4("view", hdrView[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, hdrCubemap->m_ID, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }
        captureBuffer->Unbind();

        hammersleyData->GetData().N = 20;

        int kk;
        int pos = 0;
        float p, u;

        for (int k = 0; k < hammersleyData->GetData().N; ++k)
        {
            for (p = 0.5f, kk = k, u = 0.0f; kk; p *= 0.5f, kk >>= 1)
            {
                if (kk & 1)
                {
                    u += p;
                }
            }

            float v = (k + 0.5) / hammersleyData->GetData().N;
            hammersleyData->GetData().hammersley[pos++].x = u;
            hammersleyData->GetData().hammersley[pos++].x = v;
        }
        
        hammersleyData->SetData();

        //irrComp->Activate();
        //
        //GLint srcLoc = glGetUniformLocation(irrComp->m_ID, "src");
        //glBindImageTexture(0, hdrCubemap->m_ID, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
        //glUniform1i(srcLoc, 0);
        //
        //GLint dstLoc = glGetUniformLocation(irrComp->m_ID, "dst");
        //glBindImageTexture(1, irrBlurOutput->m_ID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        //glUniform1i(dstLoc, 1);
        //
        //glDispatchCompute(512 / 16, 512 / 16, 6);
        //
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        //
        //glUseProgram(0);

        SH9Color coeffs = SphereHarmonics::GenerateLightingCoefficients(hdrCubemap->m_ID, 512);

        for (unsigned i = 0; i < 9; ++i)
        {
            harmonicData->GetData().shColor[i] = glm::vec4(coeffs.results[i], 1.0f);
        }
        harmonicData->SetData();

        return 0;
    }

    int Scene::PreRender()
    {
        lightingPass->Activate();
        lightingPass->SetInt("gPos", 0);
        lightingPass->SetInt("gNorm", 1);
        lightingPass->SetInt("gUVs", 2);
        lightingPass->SetInt("gAlbedo", 3);
        lightingPass->SetInt("gSpecular", 4);
        lightingPass->SetInt("gEntityID", 5);
        lightingPass->SetInt("gDepth", 6);

        shadowPass->Activate();
        shadowPass->SetInt("sDepth", 0);

        return 0;
    }

    int Scene::RenderEditor(EditorCamera& editorCam)
    {
        int sceneWidth = m_SceneFBO->GetSpecs().s_Width;
        int sceneHeight = m_SceneFBO->GetSpecs().s_Height;

        // G-Buffer pass
        glClearColor(0.1f, 1.0f, 0.5f, 1.0f);
        gBuffer->Activate();

        gBuffer->ClearAttachment(5, -1.0f, GL_FLOAT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // For all meshes...
        auto obj = m_Registry.view<TransformComponent, MeshComponent>();
        for (auto entity : obj)
        {
            auto [objTr, mesh] = obj.get<TransformComponent, MeshComponent>(entity);

            // Update and render them normally
            objTr.Update();
            mesh.Draw(objTr.GetTransform(), editorCam.GetViewMatrix(), 
                editorCam.GetProjection(), *geometryPass, false, (int)entity);
        }

        gBuffer->Unbind();

        // Shadow Pass
        // IMPORTANT TO DO THIS: Color will blend with BG if this is anything else but vec4(0)
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        sBuffer->Activate();
  
        glCullFace(GL_FRONT);

        // For all lights...
        auto v = m_Registry.view<TransformComponent, DirectionLightComponent>();
        for (auto entity : v)
        {
            auto [transform, light] = v.get<TransformComponent, DirectionLightComponent>(entity);

            // Update light matrices + transform
            transform.Update();
            light.Update(transform.GetTranslation(), transform.GetRotation());

            shadowPass->Activate();
            shadowPass->SetFloat("nearP", light.GetNear());
            shadowPass->SetFloat("farP", light.GetFar());
            shadowPass->SetFloat("usePersp", light.GetPerspectiveInUse());
            glUseProgram(0);

            // For all meshes...
            auto obj = m_Registry.view<TransformComponent, MeshComponent>();
            for (auto entity : obj)
            {
                auto [objTr, mesh] = obj.get<TransformComponent, MeshComponent>(entity);

                // Update and render them relative to the light
                objTr.Update();
                mesh.Draw(objTr.GetTransform(), light.GetViewMatrix(), light.GetProjectionMatrix(), *shadowPass, false);
            }
        }
        sBuffer->Unbind();


        // Lighting pass
        glClearColor(0.1f, 1.0f, 0.5f, 1.0f);
        m_SceneFBO->Activate();

        m_SceneFBO->ClearAttachment(1, -1.0f, GL_FLOAT);
        
        // Blur the shader using a convolution filter
        memset(kernelData->GetData().weights, 0, sizeof(glm::vec4) * 101);

        // Build the kernel weights
        for (int i = 0; i <= kernelSize * kernelSize; ++i)
        {
            int halfWidth = (kernelSize * kernelSize) / 2;

            int idx = i - halfWidth;
            kernelData->GetData().weights[i].x =
                Gaussian(idx, static_cast<float>(gaussianWeight));
        }

        // Normalize the kernel weights so all values sum up to 1
        float sum = 0.0f;
        for (int i = 0; i <= kernelSize * kernelSize; ++i)
        {
            sum += kernelData->GetData().weights[i].x;
        }

        for (int i = 0; i <= kernelSize * kernelSize; ++i)
        {
            kernelData->GetData().weights[i].x /= sum;
        }

        kernelData->SetData();

        computeBlur->Activate();
        computeBlur->SetInt("halfKernel", (kernelSize * kernelSize) / 2);

        GLint srcLoc = glGetUniformLocation(computeBlur->m_ID, "src");
        glBindImageTexture(0, sDepthMap->m_ID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glUniform1i(srcLoc, 0);

        GLint dstLoc = glGetUniformLocation(computeBlur->m_ID, "dst");
        glBindImageTexture(1, blurOutput->m_ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glUniform1i(dstLoc, 1);

        glDispatchCompute(2048 / 128, 2048, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glUseProgram(0);

        // Render the scene normally
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        lightingPass->Activate();

        // G-Buffer textures
        for (unsigned i = 0; i < 7; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, gTextures[i]->m_ID);
        }

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, blurOutput->m_ID);

        glm::mat4 matB = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f))
            * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

        // For all lights...(TODO: correct?)
        auto lView = m_Registry.view<TransformComponent, DirectionLightComponent>();
        for (auto entity : lView)
        {
            auto [transform, light] = lView.get<TransformComponent, DirectionLightComponent>(entity);

            // Update light matrices + transform
            transform.Update();
            light.Update(transform.GetTranslation(), transform.GetRotation());

            lightingPass->SetInt("uShadowMap", 7);
            lightingPass->SetMat4("worldToLightMat", matB * (light.GetProjectionMatrix() * light.GetViewMatrix()));

            lightingPass->SetVec3("lightDir", transform.Forward());
            lightingPass->SetVec3("viewPos", editorCam.GetPosition());

            lightingPass->SetInt("vWidth", sceneWidth);
            lightingPass->SetInt("vHeight", sceneHeight);

            RenderQuad();
        }

        // copy depth information from the gBuffer to the default framebuffer (for the skybox, so that it doesn't overlap the FSQ)
        // (also do it for the local lights so that they're not overlapped by the skybox)
        // glBlitFramebuffer depends on whether or not editor mode is enabled because glViewport changes
        // the dimensions of the FSQ
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->GetID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_SceneFBO->GetID()); // write to scene FBO
        glBlitFramebuffer(0, 0, gBuffer->GetSpecs().s_Width, gBuffer->GetSpecs().s_Height,
            0, 0, m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, m_SceneFBO->GetID());

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        // Render local lights
        {
            auto view = m_Registry.view<TransformComponent, PointLightComponent>();
            for (auto entity : view)
            {
                auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);

                transform.Scale(glm::vec3(light.GetRange()));
                transform.Update();

                light.UpdateShader("pos", Vector3(transform.GetTranslation()),
                    "color", Vector4(light.GetColor()),
                    "eyePos", Vector3(editorCam.GetPosition()),
                    "range", light.GetRange(),
                    "intensity", light.GetIntensity(),
                    "vWidth", sceneWidth,
                    "vHeight", sceneHeight);

                light.Draw(transform.GetTransform(), editorCam.GetViewMatrix(), editorCam.GetProjection());
            }
        }

        //RenderSkybox(editorCam.GetViewMatrix(), editorCam.GetProjection());
        RenderHDRMap(editorCam.GetViewMatrix(), editorCam.GetProjection());

        // Render directional lights
        {
            auto view = m_Registry.view<TransformComponent, DirectionLightComponent>();
            for (auto entity : view)
            {
                auto [transform, light] = view.get<TransformComponent, DirectionLightComponent>(entity);

                light.Draw(transform.GetTranslation(), transform.Forward(),
                    editorCam.GetViewMatrix(), editorCam.GetProjection());
            }
        }

        // Intentional - this is for mouse picking
        // UPDATE: This was changed to the G-Buffer texture, but it's
        // being bound in the ReadPixel function
        m_SceneFBO->Unbind();

        return 0;
    }

    int Scene::PostRender()
    {
        return 0;
    }

    void Scene::OnViewportResize(uint32_t w, uint32_t h)
    {
        if (_windowWidth == w && _windowHeight == h)
        {
            return;
        }

        _windowWidth = w;
        _windowHeight = h;

        m_SceneFBO->Resize(static_cast<uint32_t>(_windowWidth), static_cast<uint32_t>(_windowHeight));

        return;
    }

    void Scene::CleanUp()
    {
        return;
    }

    void Scene::UpdateEditor(DeltaTime dt, EditorCamera& edCam)
    {
        m_DT = dt.GetSeconds();

        PreRender();
        RenderEditor(edCam);
        PostRender();
    }

    void Scene::OnImGuiRender()
    {
        ImGui::Begin("Shadow Debugging");
        ImGui::SliderInt("Gaussian Blur Amount", &gaussianWeight, 1, 50);

        ImGui::Text("Shadow Map");
        ImGui::Image((void*)(intptr_t)sDepthMap->m_ID, ImVec2 { 255, 255 },
            ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::Text("Blurred Map");
        ImGui::Image((void*)(intptr_t)blurOutput->m_ID, ImVec2 { 255, 255 },
            ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::Separator();

        if (ImGui::Button("Reload Shadow Shaders"))
        {
            ReloadShaders();
        }

        ImGui::End();
    }

    void Scene::OnEvent(Event& e)
    {
        //m_Camera.OnEvent(e);
    }

    void Scene::RenderSkybox(glm::mat4 view, glm::mat4 proj)
    {
        int sceneWidth = m_SceneFBO->GetSpecs().s_Width;
        int sceneHeight = m_SceneFBO->GetSpecs().s_Height;

        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // change depth function so depth test passes when values are equal to depth buffer's content
        glDepthFunc(GL_LEQUAL);

        skyboxShader->Activate();

        skyboxShader->SetMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader->SetMat4("projection", proj);

        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->m_ID);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glUseProgram(0);

        glDepthFunc(GL_LESS);
    }

    void Scene::RenderQuad()
    {
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void Scene::RenderHDRMap(glm::mat4 view, glm::mat4 proj)
    {
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // change depth function so depth test passes when values are equal to depth buffer's content
        glDepthFunc(GL_LEQUAL);

        hdrEnvironment->Activate();

        hdrEnvironment->SetMat4("projection", proj);
        hdrEnvironment->SetMat4("view", view);

        hdrEnvironment->SetInt("environmentMap", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, hdrCubemap->m_ID);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
}
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

//#include "stb_image.h"

unsigned currLights = 4;
int currLocalLights = NUM_LIGHTS;
bool useMomentShadows = true;
float nearPlane = 0.1f, farPlane = 100.0f;
float lightW = 10.0f, lightH = 10.0f;

int kernelSize = 10;

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
        m_Camera = Camera(glm::vec3(-6.0f, 1.0f, 0.0f));
        InitMembers();

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

        Init();
    }

    Scene::Scene(int windowWidth, int windowHeight)
        : _windowWidth(windowWidth)
        , _windowHeight(windowHeight)
    {
        m_Camera = Camera(glm::vec3(-6.0f, 1.0f, 0.0f));
        InitMembers();

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

        Init();
    }

    void Scene::InitMembers()
    {
        matrixData = new UniformBuffer<World>(0);
        kernelData = new UniformBuffer<BlurKernel>(3);

        // CHOLESKY MATH TEST
        //float zTest = 0.847975643f;
        float zTest = 0.10808f - (0.5f * 0.001f);
        float depth = 0.04955f;
        float alpha = 1 * powf(10.0f, -3.0f);
        
        glm::vec3 output = glm::vec3(1.0f, zTest, zTest * zTest);
        glm::vec4 bVec = glm::vec4(depth, powf(depth, 2), powf(depth, 3), powf(depth, 4));
        
        glm::vec4 b_ = (1.0f - alpha) * bVec + alpha * glm::vec4(0.5f);
        
        glm::vec3 col1 = glm::vec3(1, b_.x, b_.y);
        glm::vec3 col2 = glm::vec3(b_.x, b_.y, b_.z);
        glm::vec3 col3 = glm::vec3(b_.y, b_.z, b_.w);
        
        glm::mat3 testMat = glm::mat3(col1, col2, col3);
        
        glm::vec3 choleskyVec = Cholesky(1.0f, b_.x, b_.y, b_.y, b_.z, b_.w, output.x, output.y, output.z);
        glm::vec3 choleskyTwoVec = Cholesky(b_, zTest);
        
        glm::vec3 res = testMat * choleskyVec;
        glm::vec3 resres = testMat * choleskyTwoVec;

        glm::vec3 cramersTest = Cramers(col1, col2, col3, output);
        glm::vec3 res2 = testMat * cramersTest;
        
        // These should equal to each other
        glm::vec2 quadr = Quadratic(cramersTest.z, cramersTest.y, cramersTest.x);
        glm::vec2 quadr2 = Quadratic(choleskyVec.z, choleskyVec.y, choleskyVec.x);
        glm::vec2 quadr3 = Quadratic(choleskyTwoVec);

        // These should both equal 0
        float quadrTest = choleskyVec.z * powf(quadr2.x, 2.0f) + choleskyVec.y * quadr2.x + choleskyVec.x;
        float quadrTest2 = choleskyVec.z * powf(quadr2.y, 2.0f) + choleskyVec.y * quadr2.y + choleskyVec.x;
        
        float shadowVal = ShadowTest(quadr2.x, quadr2.y, zTest, b_);
        float shadowVal2 = Hamburger(zTest, quadr2, b_);
        float shadowVal3 = Hausdorff(zTest, quadr2, b_);

        // END TEST
    }

    void Scene::ReloadShaders()
    {
        delete lightingPass;
        delete basicShadows;
        
        if (useMomentShadows)
        {
            lightingPass = new Shader(false, "Shadows/Moment/Diffuse.vert", "Shadows/Moment/Diffuse.frag");
            basicShadows = new Shader(false, "Shadows/Moment/Shadows.vert", "Shadows/Moment/Shadows.frag");
        }
        else
        {
            lightingPass = new Shader(false, "Shadows/Basic/Diffuse.vert", "Shadows/Basic/Diffuse.frag");
            basicShadows = new Shader(false, "Shadows/Basic/Shadows.vert", "Shadows/Basic/Shadows.frag");
        }
    }

    void Scene::GenerateLocalLights()
    {
        for (unsigned i = 0; i < MAX_LIGHTS; ++i)
        {
            float xPos = RandomNum(minX, maxX);
            float yPos = RandomNum(minY, maxY);
            float zPos = RandomNum(minZ, maxZ);
            float range = RandomNum(minRange, maxRange);
            float mult = RandomNum(5.0f, 5.0f);

            float intensity = RandomNum(1.0f, 10.0f);

            float r = RandomNum(0.0f, 1.0f);
            float g = RandomNum(0.0f, 1.0f);
            float b = RandomNum(0.0f, 1.0f);

            localLights[i].pos = glm::vec4(0.0f, 0.0f, 0.0f, 9.0f);
            localLights[i].color = glm::vec4(r, g, b, 1.0f);
            localLights[i].options.x = intensity;
            localLights[i].options.y = 1.0f;
        }
    }

    Scene::~Scene()
    {
        CleanUp();
    }

    int Scene::Init()
    {
        if (useMomentShadows)
        {
            lightingPass = new Shader(false, "Shadows/Moment/Diffuse.vert", "Shadows/Moment/Diffuse.frag");
            basicShadows = new Shader(false, "Shadows/Moment/Shadows.vert", "Shadows/Moment/Shadows.frag");
        }
        else
        {
            lightingPass = new Shader(false, "Shadows/Basic/Diffuse.vert", "Shadows/Basic/Diffuse.frag");
            basicShadows = new Shader(false, "Shadows/Basic/Shadows.vert", "Shadows/Basic/Shadows.frag");
        }

        //debugShadows = new Shader(false, "Shadows/DebugShadows.vert", "Shadows/DebugShadows.frag");
        computeBlur = new Shader(false, "Shadows/ConvolutionBlur.cmpt");

        auto shirokoTex = std::make_pair(Texture("Content/Assets/Models/BA/Shiroko/Texture2D/Shiroko_Original_Weapon.png", GL_LINEAR, GL_REPEAT), "diffTex");
        auto arisTex = std::make_pair(Texture("Content/Assets/Models/BA/Aris/Texture2D/Aris_Original_Weapon.png", GL_LINEAR, GL_REPEAT), "diffTex");
        auto hoshinoTex = std::make_pair(Texture("Content/Assets/Models/BA/Hoshino/Texture2D/Hoshino_Original_Weapon.png", GL_LINEAR, GL_REPEAT), "diffTex");
        auto ground = std::make_pair(Texture("Materials/Textures/metal_roof_diff_512x512.png"), "diffTex");

        // gBuffer textures (position, normals, UVs, albedo (diffuse), specular, depth)
        //for (unsigned i = 0; i < 5; ++i)
        //{
        //    gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_RGBA16F, GL_RGBA, nullptr,
        //        GL_NEAREST, GL_CLAMP_TO_EDGE));
        //}
        //gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, nullptr,
        //    GL_NEAREST, GL_REPEAT, GL_FLOAT));
        if (useMomentShadows)
        {
            sDepthMap = new Texture(2048, 2048, GL_RGBA32F, GL_RGBA, nullptr,
                GL_NEAREST, GL_CLAMP_TO_BORDER, GL_UNSIGNED_BYTE);
        }
        else
        {
            sDepthMap = new Texture(2048, 2048, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, nullptr,
                GL_NEAREST, GL_CLAMP_TO_BORDER, GL_FLOAT);
        }

        blurOutput = new Texture(2048, 2048, GL_RGBA32F, GL_RGBA, nullptr,
            GL_NEAREST, GL_CLAMP_TO_BORDER, GL_FLOAT);

        // gBuffer FBO
        //gBuffer = new Framebuffer(_windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //gBuffer->Bind();
        //for (unsigned i = 0; i < gTextures.size() - 1; ++i)
        //{
        //    gBuffer->AttachTexture(GL_COLOR_ATTACHMENT0 + i, *gTextures[i]);
        //}
        //gBuffer->DrawBuffers();
        //
        //gBuffer->AttachTexture(GL_DEPTH_ATTACHMENT, *gTextures[gTextures.size() - 1]);
        //gBuffer->Unbind();

        GenerateLocalLights();

        // Scene FBO (for the editor)
        m_SceneFBO = new Framebuffer(_windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_SceneFBO->Bind();
        m_SceneFBO->AllocateAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA32F, GL_RGBA, GL_UNSIGNED_BYTE);
        m_SceneFBO->AllocateAttachTexture(GL_COLOR_ATTACHMENT1, GL_R32I, GL_RED_INTEGER, GL_UNSIGNED_BYTE);
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

        if (useMomentShadows)
        {
            sBuffer->AttachTexture(GL_COLOR_ATTACHMENT0, *sDepthMap);
            sBuffer->DrawBuffers();
            sBuffer->AllocateAttachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
        }
        else
        {
            sBuffer->AllocateAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA32F, GL_RGBA, GL_UNSIGNED_BYTE);
            sBuffer->DrawBuffers();
            sBuffer->AttachTexture(GL_DEPTH_ATTACHMENT, *sDepthMap);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Uh oh! Shadow FBO is incomplete!" << std::endl;
            sBuffer->Unbind();
            return -1;
        }

        sBuffer->Unbind();

        return 0;
    }

    int Scene::PreRender()
    {
        //lightingPass->Activate();
        //lightingPass->SetInt("gPos", 0);
        //lightingPass->SetInt("gNorm", 1);
        //lightingPass->SetInt("gUVs", 2);
        //lightingPass->SetInt("gAlbedo", 3);
        //lightingPass->SetInt("gSpecular", 4);
        //lightingPass->SetInt("gDepth", 5);

        basicShadows->Activate();
        basicShadows->SetInt("sDepth", 0);

        return 0;
    }

    int Scene::Render()
    {
        //std::cout << "1st " << m_Camera.cameraPos << std::endl;
        //std::cout << "2nd " << m_Camera.cameraPos + m_Camera.front << std::endl;
        //std::cout << "3rd " << m_Camera.up << std::endl;
        //std::cout << "front " << m_Camera.front << std::endl;
        //std::cout << "up " << m_Camera.up << std::endl;

        int sceneWidth = m_SceneFBO->GetSpecs().s_Width;
        int sceneHeight = m_SceneFBO->GetSpecs().s_Height;

        //glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        //glEnable(GL_DEPTH_TEST);
        //glDisable(GL_BLEND);

        // Shadow Pass
        // Step 1: Light POV to FBO
        glm::mat4 lightProj = glm::ortho(-lightW, lightW, -lightH, lightH, nearPlane, farPlane);
        glm::mat4 lightView = glm::lookAt(
            glm::vec3(10.2296f, 6.20311f, -14.2213f),
            glm::vec3(9.64084f, 5.91074f, -13.4677f), 
            glm::vec3(-0.180002f, 0.956306f, 0.230392f));

        glm::mat4 lightSpaceMat = lightProj * lightView;
        glm::mat4 matB = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));

        glCullFace(GL_FRONT);
        // Render models relative to the shadow shader
        sBuffer->Activate();
        {
            auto view = m_Registry.view<TransformComponent, MeshComponent>();
            for (auto entity : view)
            {
                auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);
        
                transform.Update();
                basicShadows->Activate();
                basicShadows->SetFloat("nearP", nearPlane);
                basicShadows->SetFloat("farP", farPlane);

                glUseProgram(0);
                mesh.Draw(transform.GetTransform(), lightView, lightProj, *basicShadows, false);
            }
        }
        sBuffer->Unbind();

        glClearColor(0.1f, 1.0f, 0.5f, 1.0f);
        m_SceneFBO->Activate();

        m_SceneFBO->ClearAttachment(1, -1);

        if (useMomentShadows)
        {
            // Step 2: Blur the shader using a convolution filter
            memset(kernelData->GetData().weights, 0, sizeof(glm::vec4) * 101);

            // Build the kernel weights
            for (int i = 0; i <= kernelSize * kernelSize; ++i)
            {
                int halfWidth = (kernelSize * kernelSize) / 2;

                int idx = i - halfWidth;
                kernelData->GetData().weights[i].x = 
                    Gaussian(idx, (kernelSize % 2 == 0 ? halfWidth : (kernelSize + 1)) / 2);
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
        }


        // Step 3: Render the scene normally
        glCullFace(GL_BACK);

        auto view = m_Registry.view<TransformComponent, LightComponent>();
        for (auto entity : view)
        {
            auto [transform, light] = view.get<TransformComponent, LightComponent>(entity);

            transform.Update();
            //light.Draw(transform.GetTranslation(), transform.GetRotation(),
            //    m_Camera.View(),
            //    m_Camera.Perspective(sceneWidth, sceneHeight));
        }
        
        lightingPass->Activate();

        glActiveTexture(GL_TEXTURE0);

        if (useMomentShadows)
        {
            glBindTexture(GL_TEXTURE_2D, blurOutput->m_ID);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, sDepthMap->m_ID);
        }

        lightingPass->SetInt("uShadowMap", 0);
        lightingPass->SetMat4("shadowMatrix", matB * lightSpaceMat);
        lightingPass->SetInt("vWidth", sceneWidth);
        lightingPass->SetInt("vHeight", sceneHeight);

        {
            auto view = m_Registry.view<TransformComponent, MeshComponent>();
            for (auto entity : view)
            {
                auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);

                transform.Update();
                mesh.Draw(transform.GetTransform(), m_Camera.View(), 
                    m_Camera.Perspective(sceneWidth, sceneHeight), *lightingPass, false, (int)entity);
            }
        }

        //m_SceneFBO->Unbind();

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
        ImGui::Begin("Shadow Debugging");

        ImGui::SliderFloat("Light Width", &lightW, 1.0f, 1000.0f);
        ImGui::SliderFloat("Light Height", &lightH, 1.0f, 1000.0f);
        ImGui::Separator();
        ImGui::SliderFloat("Light Max Depth", &farPlane, 1.0f, 100.0f);
        ImGui::SliderFloat("Light Min Depth", &nearPlane, 0.0f, farPlane);
        ImGui::Separator();
        ImGui::SliderInt("Kernel Size", &kernelSize, 1, 10);

        ImGui::Text("Shadow Map");
        ImGui::Image((void*)(intptr_t)sDepthMap->m_ID, ImVec2 { 255, 255 },
            ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::Text("Blurred Map");
        ImGui::Image((void*)(intptr_t)blurOutput->m_ID, ImVec2 { 255, 255 },
            ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

        ImGui::Separator();

        if (ImGui::Button("Move to light position and direction"))
        {
            m_Camera.cameraPos = glm::vec3(10.2296f, 6.20311f, -14.2213f);
            m_Camera.front = glm::vec3(-0.58876f, -0.292372f, 0.753578f);
            m_Camera.up = glm::vec3(-0.180002f, 0.956305f, 0.230392f);
        }

        if (ImGui::Button("Reload Shadow Shaders"))
        {
            ReloadShaders();
        }

        if (ImGui::Button("Switch between regular/moment"))
        {
            useMomentShadows = !useMomentShadows;
        }

        ImGui::End();
    }

    void Scene::OnEvent(Event& e)
    {
        //m_Camera.OnEvent(e);
    }

    void Scene::RenderLocalLights()
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        localLight->Activate();
        localLight->SetVec3("eyePos", m_Camera.cameraPos);

        localLight->SetInt("vWidth", _windowWidth);
        localLight->SetInt("vHeight", _windowHeight);

        glUseProgram(0);

        //for (unsigned i = 0; i < static_cast<unsigned>(currLocalLights); ++i)
        //{
        //    localLightData->GetData().pos = localLights[i].pos;
        //    localLightData->GetData().color = localLights[i].color;
        //    localLightData->GetData().options = localLights[i].options;
        //    localLightData->SetData();
        //
        //    sphere->Update(0.0f, glm::vec3(localLights[i].pos.w * localLights[i].options.y), glm::vec3(localLights[i].pos));
        //
        //    sphere->Draw(localLight->m_ID, m_Camera.View(), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height));
        //}
        //
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        //glEnable(GL_DEPTH_TEST);
        //glDisable(GL_BLEND);
        //
        //for (unsigned i = 0; i < static_cast<unsigned>(currLocalLights); ++i)
        //{
        //    localLightData->GetData().pos = localLights[i].pos;
        //    localLightData->GetData().color = localLights[i].color;
        //    localLightData->SetData();
        //
        //    sphere->Update(0.0f, glm::vec3(1.0f), glm::vec3(localLights[i].pos));
        //    sphere->Draw(flatShader->m_ID, m_Camera.View(), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height));
        //
        //    if (m_DisplayDebugRanges)
        //    {
        //        sphere->Update(0.0f, glm::vec3(localLights[i].pos.w * localLights[i].options.y), glm::vec3(localLights[i].pos));
        //        sphere->Draw(flatShader->m_ID, m_Camera.View(), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height), {}, GL_LINES);
        //    }
        //}
    }

    void Scene::RenderSkybox()
    {
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

        matrixData->GetData().view = glm::mat4(glm::mat3(m_Camera.View()));
        matrixData->SetData();

        //skybox->Update(0.0f, glm::vec3(50.0f), (m_Camera.cameraPos + skybox->getModelCentroid()) / glm::vec3(50.0f));
        //skybox->Draw(skyboxShader->m_ID, glm::mat4(glm::mat3(m_Camera.View())), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height), skyboxTextures, GL_TRIANGLES);

        glDepthFunc(GL_LESS);
    }

    void Scene::RenderQuad()
    {
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
}
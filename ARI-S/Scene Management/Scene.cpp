#include <arpch.h>

#include "Scene.h"
#include "Entity.h"

#include <vec3.hpp>
#include <glad/glad.h>

#include <gtc/matrix_access.hpp>
#include <gtc/type_ptr.hpp>

#include "imgui.h"

#include "Math/Vector.h"

//#include "stb_image.h"

unsigned currLights = 4;
int currLocalLights = NUM_LIGHTS;

namespace ARIS
{
    float RandomNum(float min, float max)
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
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
        lightingPass = new Shader(false, "Shadows/DiffuseWithShadows.vert", "Shadows/DiffuseWithShadows.frag");
        basicShadows = new Shader(false, "Shadows/BasicShadows.vert", "Shadows/BasicShadows.frag");

        //Texture t1 = Texture("Content/Assets/Models/BA/Shiroko/Texture2D/Shiroko_Original_Weapon.png", GL_LINEAR, GL_REPEAT);
        //Texture t2 = Texture("Content/Assets/Models/BA/Shiroko/Texture2D/Shiroko_Original_Weapon.png", GL_LINEAR, GL_REPEAT);

        // Object textures
        //textures.push_back(std::make_pair(t1, "diffTex"));
        //textures.push_back(std::make_pair(t2, "specTex"));
        //
        //groundTextures.push_back(std::make_pair(new Texture("Materials/Textures/metal_roof_diff_512x512.png", GL_LINEAR, GL_REPEAT), "diffTex"));
        //groundTextures.push_back(std::make_pair(new Texture("Materials/Textures/metal_roof_spec_512x512.png", GL_LINEAR, GL_REPEAT), "specTex"));

        // gBuffer textures (position, normals, UVs, albedo (diffuse), specular, depth)
        //for (unsigned i = 0; i < 5; ++i)
        //{
        //    gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_RGBA16F, GL_RGBA, nullptr,
        //        GL_NEAREST, GL_CLAMP_TO_EDGE));
        //}
        //gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, nullptr,
        //    GL_NEAREST, GL_REPEAT, GL_FLOAT));

        sDepthMap = new Texture(2048, 2048, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, nullptr,
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

        // Shadow FBO 
        sBuffer = new Framebuffer(2048, 2048, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sBuffer->Bind();
        sBuffer->AllocateAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA, GL_UNSIGNED_BYTE);
        sBuffer->DrawBuffers();
        sBuffer->AttachTexture(GL_DEPTH_ATTACHMENT, *sDepthMap);

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
        int sceneWidth = m_SceneFBO->GetSpecs().s_Width;
        int sceneHeight = m_SceneFBO->GetSpecs().s_Height;

        glClearColor(0.1f, 1.0f, 0.5f, 1.0f);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // Shadow Pass
        // Step 1: Light POV to FBO
        glm::mat4 lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
        glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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
                mesh.SetTextures(textures);
                mesh.Draw(transform.GetTransform(), lightView, lightProj, *basicShadows, false);
            }
        }
        sBuffer->Unbind();
        
        
        // Step 2: Render the scene normally
        glCullFace(GL_BACK);
        lightingPass->Activate();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sDepthMap->m_ID);

        lightingPass->SetInt("uShadowMap", 0);
        lightingPass->SetMat4("shadowMatrix", matB * lightSpaceMat);

        m_SceneFBO->Activate();
        {
            auto view = m_Registry.view<TransformComponent, MeshComponent>();
            for (auto entity : view)
            {
                auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);

                transform.Update();
                mesh.SetTextures(textures);
                mesh.Draw(transform.GetTransform(), m_Camera.View(), m_Camera.Perspective(sceneWidth, sceneHeight), *lightingPass, false);
            }
        }
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
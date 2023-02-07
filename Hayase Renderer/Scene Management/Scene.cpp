#include <hyspch.h>

#include "Scene.h"
#include "Entity.h"

#include <vec3.hpp>
#include <glad/glad.h>

#include <gtc/matrix_access.hpp>
#include <gtc/type_ptr.hpp>

#include "imgui.h"

//#include "stb_image.h"

unsigned currLights = 4;
int currLocalLights = NUM_LIGHTS;

namespace Hayase
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

            localLights[i].pos = glm::vec4(xPos, yPos, zPos, range);
            localLights[i].color = glm::vec4(r, g, b, 1.0f);
            localLights[i].options.x = intensity;
            localLights[i].options.y = mult;
        }
    }

    Scene::~Scene()
    {
        CleanUp();
    }

    int Scene::Init()
    {
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

        return 0;
    }

    int Scene::PreRender()
    {
        return 0;
    }

    int Scene::Render()
    {
        glClearColor(0.1f, 1.0f, 0.5f, 1.0f);
        m_SceneFBO->Activate();

        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // render
        {
            auto view = m_Registry.view<TransformComponent, MeshComponent>();
            for (auto entity : view)
            {
                auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);

                transform.Update();
                mesh.Draw(transform.GetTransform(), m_Camera.View(), m_Camera.Perspective(_windowWidth, _windowHeight));
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
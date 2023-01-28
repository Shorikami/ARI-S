#include <hyspch.h>

#include "Deferred.h"

#include <vec3.hpp>
#include <glad/glad.h>

#include <gtc/matrix_access.hpp>
#include <gtc/type_ptr.hpp>

#include "imgui.h"

#include "Application.h"

#include "stb_image.h"

int currLights = 4;
int currLocalLights = NUM_LIGHTS;

namespace Hayase
{
    float RandomNum(float min, float max)
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    Deferred::~Deferred()
    {

    }

    void Deferred::OnAttach()
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

        Init();
    }

    void Deferred::OnDetach()
    {
        CleanUp();
    }

    void Deferred::Update(DeltaTime dt)
    {
        float time = dt.GetSeconds();

        // update first...
        m_Camera.Update(dt);

        // ... then render
        Display();
    }

    void Deferred::OnEvent(Event& e)
    {
        m_Camera.OnEvent(e);
    }

    void Deferred::OnImGuiRender()
    {
        ImGui::Begin("Settings");
        ImGui::End();
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    Deferred::Deferred(int windowWidth, int windowHeight)
        : Scene(windowWidth, windowHeight)
        , Layer("Deferred Scene")
        , angleOfRotation(0.0f)
        , m_Camera(glm::vec3(-6.0f, 1.0f, 0.0f))
    {
        initMembers();
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-unused-return-value"
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    void Deferred::initMembers()
    {
        //shaderID = lineShaderID = 0;
        angleOfRotation = 0.0f;

        matrixData = new UniformBuffer<World>(0);
        lightData = new UniformBuffer<Lights>(1);
        localLightData = new UniformBuffer<LocalLight>(2);
    }
#pragma clang diagnostic pop

    void Deferred::ReloadShaders()
    {
        // gross, pls make a library or something
        delete geometryPass;
        delete lightingPass;
        delete localLight;
        delete flatShader;
        delete skyboxShader;

        flatShader = new Shader(false, "FlatShader.vert", "FlatShader.frag");

        geometryPass = new Shader(false, "Deferred/GeometryPass.vert", "Deferred/GeometryPass.frag");
        lightingPass = new Shader(false, "Deferred/LightingPass.vert", "Deferred/LightingPass.frag");
        localLight = new Shader(false, "Deferred/LocalLight.vert", "Deferred/LocalLight.frag");

        skyboxShader = new Shader(false, "Reflections/Skybox.vert", "Reflections/Skybox.frag");
    }

    void Deferred::GenerateLocalLights()
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

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    void Deferred::CleanUp()
    {
        delete geometryPass;
        delete lightingPass;
        delete localLight;
        delete flatShader;
        delete skyboxShader;

        for (unsigned i = 0; i < models.size(); ++i)
        {
            delete models[i];
        }

        cube->Cleanup();
        sphere->Cleanup();
        skybox->Cleanup();

        delete cube;
        delete sphere;
        delete skybox;
    }


    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int Deferred::Init()
    {
        flatShader = new Shader(false, "FlatShader.vert", "FlatShader.frag");

        geometryPass = new Shader(false, "Deferred/GeometryPass.vert", "Deferred/GeometryPass.frag");
        lightingPass = new Shader(false, "Deferred/LightingPass.vert", "Deferred/LightingPass.frag");
        localLight = new Shader(false, "Deferred/LocalLight.vert", "Deferred/LocalLight.frag");

        skyboxShader = new Shader(false, "Reflections/Skybox.vert", "Reflections/Skybox.frag");

        // Object textures
        textures.push_back(std::make_pair(new Texture("Materials/Models/BA/Shiroko/Texture2D/Shiroko_Original_Weapon.png", GL_LINEAR, GL_REPEAT), "diffTex"));
        textures.push_back(std::make_pair(new Texture("Materials/Models/BA/Shiroko/Texture2D/Shiroko_Original_Weapon.png", GL_LINEAR, GL_REPEAT), "specTex"));

        groundTextures.push_back(std::make_pair(new Texture("Materials/Textures/metal_roof_diff_512x512.png", GL_LINEAR, GL_REPEAT), "diffTex"));
        groundTextures.push_back(std::make_pair(new Texture("Materials/Textures/metal_roof_spec_512x512.png", GL_LINEAR, GL_REPEAT), "specTex"));

        // gBuffer textures (position, normals, UVs, albedo (diffuse), specular, depth)
        for (unsigned i = 0; i < 5; ++i)
        {
            gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_RGBA16F, GL_RGBA, nullptr,
                GL_NEAREST, GL_CLAMP_TO_EDGE));
        }
        gTextures.push_back(new Texture(_windowWidth, _windowHeight, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, nullptr,
            GL_NEAREST, GL_REPEAT, GL_FLOAT));
        
        // gBuffer FBO
        

        gBuffer = new Framebuffer(_windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gBuffer->Bind();
        for (unsigned i = 0; i < gTextures.size() - 1; ++i)
        {
            gBuffer->AttachTexture(GL_COLOR_ATTACHMENT0 + i, *gTextures[i]);
        }
        gBuffer->DrawBuffers();

        gBuffer->AttachTexture(GL_DEPTH_ATTACHMENT, *gTextures[gTextures.size() - 1]);
        gBuffer->Unbind();

        // skybox textures
        {
            std::string common = "Materials/Textures/Skyboxes/Day/skybox_";
        
            skyboxTextures.push_back(std::make_pair(new Texture(common + "right.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[0]"));
            skyboxTextures.push_back(std::make_pair(new Texture(common + "left.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[1]"));
            skyboxTextures.push_back(std::make_pair(new Texture(common + "top.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[2]"));
            skyboxTextures.push_back(std::make_pair(new Texture(common + "bottom.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[3]"));
            skyboxTextures.push_back(std::make_pair(new Texture(common + "front.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[4]"));
            skyboxTextures.push_back(std::make_pair(new Texture(common + "back.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[5]"));
        }

        std::vector<glm::vec3> positions =
        {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(8.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 8.0f),
            glm::vec3(8.0f, 0.0f, 8.0f)
        };

        std::vector<std::pair<glm::vec3, Model::RotationAxis>> quadPositions =
        {
            std::make_pair(glm::vec3(3.0f, -2.0f, 5.0f), Model::RotationAxis::yAxis),
            std::make_pair(glm::vec3(13.0f, 8.0f, 5.0f), Model::RotationAxis::zAxis),
            std::make_pair(glm::vec3(3.0f, 8.0f, 15.0f), Model::RotationAxis::xAxis)
        };

        for (unsigned i = 0; i < 4; ++i)
        {
            models.push_back(new Model("Materials/Models/BA/Shiroko/Mesh/Shiroko_Original_Weapon.obj", positions[i],
                glm::vec3(10.0f), 0.0f, Model::RotationAxis::yAxis, textures));
            models[i]->Name(std::string("White Fang ") + std::to_string(i + 1));
        }

        for (unsigned i = 4; i < 7; ++i)
        {
            models.push_back(new Model("Materials/Models/Plane.obj", quadPositions[i - 4].first,
                glm::vec3(20.0f), 0.0f, quadPositions[i - 4].second, groundTextures));
            models[i]->Name(std::string("Ground ") + std::to_string((i - 4) + 1));
        }

        // TODO: Map out model container?
        models[5]->Rotate(glm::pi<float>() / 2.0f);
        models[6]->Rotate(glm::pi<float>() / 2.0f);
        models[6]->InvertAxis(true);

        skybox = new Mesh();
        skybox->initData();
        reader.ReadOBJFile("Materials/Models/cube.obj", skybox);

        cube = new Mesh();
        cube->initData();
        reader.ReadOBJFile("Materials/Models/cube2.obj", cube);

        sphere = Mesh::CreateSphere(0.08f, 16);

        skybox->GenerateBuffers();
        sphere->GenerateBuffers();
        cube->GenerateBuffers();

        // Regular light UBO (Obsolete?)
        {
            Lights& lightUBO = lightData->GetData();

            for (unsigned i = 0; i < MAX_LIGHTS; ++i)
            {
                float xPos = RandomNum(-4.0f, 10.0f);
                float zPos = RandomNum(-4.0f, 10.0f);

                lightUBO.lightPos[i] = glm::vec4(xPos, 0.0f, zPos, 1.0f);

                float r = RandomNum(0.0f, 1.0f);
                float g = RandomNum(0.0f, 1.0f);
                float b = RandomNum(0.0f, 1.0f);

                lightUBO.lightColor[i] = glm::vec4(r, g, b, 1.0f);

                lightUBO.ambient[i] = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
                lightUBO.diffuse[i] = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
                lightUBO.specular[i] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

                // x = outer, y = inner, z = falloff, w = type
                lightUBO.lightInfo[i] = glm::vec4(80.0f, 45.0f, 10.0f, 0.0f);
            }

            lightUBO.emissive = glm::vec4(0.0f);
            lightUBO.globalAmbient = glm::vec4(0.0f, 0.0f, 26.0f / 255.0f, 1.0f);
            lightUBO.coefficients = glm::vec4(1.0f);
            lightUBO.fogColor = glm::vec4(m_BGColor, 1.0f);
            lightUBO.numLights = currLights;

            // x = use gpu, y = use normals, z = uv type
            lightUBO.modes = glm::ivec4(0, 0, 0, 0);
        }

        // Local light pass UBO
        GenerateLocalLights();

        localLightData->SetData();

        matrixData->GetData().nearFar = glm::vec2(0.1f, 20.0f);
        matrixData->SetData();

        return 0;
    }

    int Deferred::PreRender(float frame)
    {
        Lights& lightUBO = lightData->GetData();

        lightUBO.eyePos = glm::vec4(m_Camera.cameraPos, 1.0f);
        lightData->SetData();

        lightingPass->Activate();
        lightingPass->SetInt("gPos", 0);
        lightingPass->SetInt("gNorm", 1);
        lightingPass->SetInt("gUVs", 2);
        lightingPass->SetInt("gAlbedo", 3);
        lightingPass->SetInt("gSpecular", 4);
        lightingPass->SetInt("gDepth", 5);

        return 0;
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int Deferred::Render()
    {
        matrixData->GetData().proj = m_Camera.perspective();
        matrixData->GetData().view = m_Camera.view();
        matrixData->SetData();

        glClearColor(m_BGColor.x, m_BGColor.y, m_BGColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // (-_windowWidth + (EditorInfo::leftSize + EditorInfo::rightSize)) + 
        // gBuffer pass
        gBuffer->Activate();

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        geometryPass->Activate();

        for (unsigned i = 0; i < models.size(); ++i)
        {
            models[i]->Update();
            models[i]->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective());
        }

        gBuffer->Unbind();

        // lighting pass
        glClearColor(m_BGColor.x, m_BGColor.y, m_BGColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        
        lightingPass->Activate();
        for (unsigned i = 0; i < 6; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, gTextures[i]->ID);
        }
        
        lightingPass->SetVec3("viewPos", m_Camera.cameraPos);
        lightingPass->SetInt("renderOption", m_RenderOption);

        lightingPass->SetInt("vWidth", _windowWidth);
        lightingPass->SetInt("vHeight", _windowHeight);
        
        RenderQuad();

        // copy depth information from the gBuffer to the default framebuffer (for the skybox, so that it doesn't overlap the FSQ)
        // (also do it for the local lights so that they're not overlapped by the skybox)
        // glBlitFramebuffer depends on whether or not editor mode is enabled because glViewport changes
        // the dimensions of the FSQ
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->GetID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        glBlitFramebuffer(0, 0,  _windowWidth,_windowHeight, 0, 0, _windowWidth, _windowHeight, 
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        Lights& lightUBO = lightData->GetData();
        
        // Render the locations of the lighting pass lights
        if (m_DisplayLightPassLocations)
        {
            for (unsigned i = 0; i < currLights; ++i)
            {
                // flat shader relies on local light ubo; might change this
                localLightData->GetData().color = lightUBO.diffuse[i];
                localLightData->SetData();
        
                cube->Update(0.0f, glm::vec3(0.2f), glm::vec3(lightUBO.lightPos[i]));
                cube->Draw(flatShader->m_ID, m_Camera.view(), m_Camera.perspective());
            }
        }
        
        // Local light "pass" (it's being forward rendered)
        if (m_DisplayLocalLights)
        {
            RenderLocalLights();
        }
        
        // + the skybox
        if (m_DisplaySkybox)
        {
            RenderSkybox();
        }
        
        return 0;
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int Deferred::PostRender()
    {
        return 0;
    }

    void Deferred::RenderQuad()
    {
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void Deferred::RenderLocalLights()
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

        for (unsigned i = 0; i < currLocalLights; ++i)
        {
            localLightData->GetData().pos = localLights[i].pos;
            localLightData->GetData().color = localLights[i].color;
            localLightData->GetData().options = localLights[i].options;
            localLightData->SetData();

            sphere->Update(0.0f, glm::vec3(localLights[i].pos.w * localLights[i].options.y), glm::vec3(localLights[i].pos));

            sphere->Draw(localLight->m_ID, m_Camera.view(), m_Camera.perspective());
        }

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        for (unsigned i = 0; i < currLocalLights; ++i)
        {
            localLightData->GetData().pos = localLights[i].pos;
            localLightData->GetData().color = localLights[i].color;
            localLightData->SetData();

            sphere->Update(0.0f, glm::vec3(1.0f), glm::vec3(localLights[i].pos));
            sphere->Draw(flatShader->m_ID, m_Camera.view(), m_Camera.perspective());

            if (m_DisplayDebugRanges)
            {
                sphere->Update(0.0f, glm::vec3(localLights[i].pos.w * localLights[i].options.y), glm::vec3(localLights[i].pos));
                sphere->Draw(flatShader->m_ID, m_Camera.view(), m_Camera.perspective(), {}, GL_LINES);
            }
        }
    }

    void Deferred::RenderSkybox()
    {
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

        matrixData->GetData().view = glm::mat4(glm::mat3(m_Camera.view()));
        matrixData->SetData();

       //skybox->Update(0.0f, glm::vec3(50.0f), (m_Camera.cameraPos + skybox->getModelCentroid()) / glm::vec3(50.0f));
       skybox->Draw(skyboxShader->m_ID, glm::mat4(glm::mat3(m_Camera.view())), m_Camera.perspective(), skyboxTextures, GL_TRIANGLES);
       
       glDepthFunc(GL_LESS);
    }
}
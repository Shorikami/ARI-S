#include <hyspch.h>

#include "Deferred.h"

#include <vec3.hpp>
#include <glad/glad.h>

#include <gtc/matrix_access.hpp>
#include <gtc/type_ptr.hpp>

#include "imgui.h"

#include "Application.h"

#include "stb_image.h"

unsigned currLights = 4;
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
        CleanUp();
    }

    void Deferred::OnAttach()
    {
    }

    void Deferred::OnDetach()
    {
    }

    void Deferred::Update(DeltaTime dt)
    {
        m_DT = dt.GetSeconds();

        // update first...
        m_Camera.Update(m_DT);

        // ... then render
        Display();
    }

    void Deferred::OnEvent(Event& e)
    {
        m_Camera.OnEvent(e);
    }

    void Deferred::OnImGuiRender()
    {
        ImGui::Begin("FSQ Textures");
        std::vector<std::string> texNames =
        {
            "Positions",
            "Normals",
            "UVs",
            "Diffuse",
            "Specular",
            "Depth"
        };
        int imgWidth = 225;
        int imgHeight = 225;

        for (unsigned i = 0; i < 6; ++i)
        {
            ImGui::SetCursorPos(ImVec2((400 - imgWidth) * 0.5f, (100 + i * (imgHeight * 2.5f)) * 0.5f));
            ImGui::Image((void*)(intptr_t)gTextures[i]->m_ID, ImVec2((float)imgWidth, (float)imgHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Text(texNames[i].c_str());
            ImGui::Separator();
        }
        ImGui::End();

        ImGui::Begin("Debug Info");
        ImGui::Text("FPS: %.3f", m_DT.GetSeconds());
        ImGui::Separator();
        ImGui::Text("Camera X: %.2f", m_Camera.cameraPos.x);
        ImGui::Text("Camera Y: %.2f", m_Camera.cameraPos.y);
        ImGui::Text("Camera Z: %.2f", m_Camera.cameraPos.z);
        ImGui::Separator();

        ImGui::Text("Welcome to the Hayase Renderer! A few things to note:");
        ImGui::Text("- The program crashes if you minimize the window because");
        ImGui::Text("the camera perspective function divides by zero");
        ImGui::Text("I'm planning on reworking the way this editor is created");
        ImGui::Text("since I don't want to create hacky solutions to some of");
        ImGui::Text("the below problems. I'll start on the rework after I");
        ImGui::Text("submit this project");
        ImGui::Text("- Resizing the window somewhat breaks the viewport.");
        ImGui::Text("Fixing that is a high priority");
        ImGui::Text("- The FSQ textures appear like the way they do because");
        ImGui::Text("of the method I'm rendering them through glViewport");
        ImGui::Text("- .obj files are being loaded via tinyobjloader, but");
        ImGui::Text("some objects are using the OBJReader class from the");
        ImGui::Text("CS300 Framework because they lack built-in normals");
        ImGui::Text("and UVs");
        ImGui::Text("- Credits to yochan.176 on Sketchfab for the Blue");
        ImGui::Text("Archive gun models that I used in this renderer.");
        ImGui::Text("I'm keeping them in because I'm a bit lazy to find");
        ImGui::Text("and test more complex models, but the guns were also");
        ImGui::Text("good for deferred and UV generation testing purposes");
        ImGui::Separator();
        ImGui::End();

        ImGui::Begin("Scene Settings");
        if (ImGui::Button("Reload Shaders"))
        {
            ReloadShaders();
        }
        ImGui::Separator();

        ImGui::SliderFloat("Camera Near", &m_Camera.n, 0.1f, 10.0f);
        ImGui::SliderFloat("Camera Far", &m_Camera.f, 10.0f, 500.0f);
        ImGui::Separator();

        ImGui::Checkbox("Display Local Light Ranges", &m_DisplayDebugRanges);
        ImGui::Checkbox("Display Light Pass Locations", &m_DisplayLightPassLocations);
        ImGui::Checkbox("Display Local Lights", &m_DisplayLocalLights);
        ImGui::Checkbox("Display Skybox", &m_DisplaySkybox);
        ImGui::Separator();

        ImGui::Text("FSQ Rendering");
        {
            static int renderWhat = 0;
            renderWhat = m_RenderOption;
            ImGui::RadioButton("Full Deferred", &renderWhat, 0);
            ImGui::RadioButton("Positions Only", &renderWhat, 1);
            ImGui::RadioButton("Normals Only", &renderWhat, 2);
            ImGui::RadioButton("UVs Only", &renderWhat, 3);
            ImGui::RadioButton("Diffuse Only", &renderWhat, 4);
            ImGui::RadioButton("Specular Only", &renderWhat, 5);
            ImGui::RadioButton("Depth Only", &renderWhat, 6);
            m_RenderOption = renderWhat;
        }
        ImGui::Separator();

        ImGui::Text("Local Light Values");
        if (ImGui::Button("Randomize Lights"))
        {
            GenerateLocalLights();
        }

        float speed = 1.0f;

        ImGui::PushItemWidth(100.0f);

        ImGui::DragFloat("Min. X", &minX, speed, -50.0f, 50.0f);
        ImGui::SameLine(); ImGui::DragFloat("Max. X", &maxX, speed, -50.0f, 50.0f);

        ImGui::DragFloat("Min. Y", &minY, speed, -50.0f, 50.0f);
        ImGui::SameLine(); ImGui::DragFloat("Max. Y", &maxY, speed, -50.0f, 50.0f);

        ImGui::DragFloat("Min. Z", &minZ, speed, -50.0f, 50.0f);
        ImGui::SameLine(); ImGui::DragFloat("Max. Z", &maxZ, speed, -50.0f, 50.0f);
        ImGui::DragFloat("Min. Range", &minRange, speed, 1.0f, 10.0f);
        ImGui::SameLine(); ImGui::DragFloat("Max. Range", &maxRange, speed, 1.0f, 10.0f);

        ImGui::PopItemWidth();
        ImGui::End();

        ImGui::Begin("Models");
        static int selectedModel = 0;
        selectedModel = m_SelectedModelIdx;

        for (unsigned i = 0; i < models.size(); ++i)
        {
            ImGui::RadioButton(models[i]->Name().c_str(), &selectedModel, i);
        }
        m_SelectedModelIdx = selectedModel;

        ImGui::End();

        ImGui::Begin("Local Lights");
        ImGui::PushItemWidth(100.0f);
        ImGui::SliderInt("No. of Local Lights", &currLocalLights, 1, MAX_LIGHTS);
        ImGui::PopItemWidth();
        ImGui::Separator();

        for (unsigned i = 0; i < static_cast<unsigned>(currLocalLights); ++i)
        {
            ImGui::PushID(i);

            ImGui::SliderFloat3("Light Position", glm::value_ptr(localLights[i].pos), -20.0f, 20.0f);
            ImGui::SliderFloat("Light Range", &localLights[i].pos.w, 1.0f, 50.0f);
            ImGui::Separator();
            ImGui::ColorEdit4("Light Color", glm::value_ptr(localLights[i].color));
            ImGui::Separator();
            ImGui::SliderFloat("Intensity", &localLights[i].options.x, 0.0f, 10.0f);
            ImGui::SliderFloat("Range Multiplier", &localLights[i].options.y, 1.0f, 5.0f);
            ImGui::Separator();
            ImGui::Text("Total Range: %f", 0.08f * localLights[i].pos.w * localLights[i].options.y);

            ImGui::PopID();
        }
        ImGui::End();

        ImGui::Begin("Model Transformation");
        ImGui::SliderFloat3("Position", glm::value_ptr(models[m_SelectedModelIdx]->Translation()), -20.0f, 20.0f);
        ImGui::Checkbox("Invert Axis?", &models[m_SelectedModelIdx]->AxisInverted());

        static int rotAxis = 0;
        rotAxis = static_cast<int>(models[m_SelectedModelIdx]->Rotation());
        ImGui::SameLine(); ImGui::RadioButton("X Axis", &rotAxis, 0);
        ImGui::SameLine(); ImGui::RadioButton("Y Axis", &rotAxis, 1);
        ImGui::SameLine(); ImGui::RadioButton("Z Axis", &rotAxis, 2);
        models[m_SelectedModelIdx]->Rotation() = static_cast<Model::RotationAxis>(rotAxis);

        ImGui::SliderAngle("Angle", &models[m_SelectedModelIdx]->Angle(), 0.0f, 360.0f);
        ImGui::SliderFloat3("Scale", glm::value_ptr(models[m_SelectedModelIdx]->Scale()), 0.1f, 20.0f);
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

        delete gBuffer;
        delete m_SceneFBO;
    }

    void Deferred::OnViewportResize(uint32_t w, uint32_t h)
    {
        if (_windowWidth == w && _windowHeight == h)
        {
            return;
        }

        _windowWidth = w;
        _windowHeight = h;
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

        // Scene FBO (for the editor)
        m_SceneFBO = new Framebuffer(_windowWidth, _windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_SceneFBO->Bind();
        m_SceneFBO->AllocateAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA, GL_UNSIGNED_BYTE);
        m_SceneFBO->DrawBuffers();
        m_SceneFBO->AllocateAttachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_FLOAT);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Uh oh! Refresh() command is incomplete!" << std::endl;
        }

        m_SceneFBO->Unbind();

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
        matrixData->GetData().proj = m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height);
        matrixData->GetData().view = m_Camera.View();
        matrixData->SetData();

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
            models[i]->Draw(geometryPass->m_ID, m_Camera.View(), m_Camera.Perspective(gBuffer->GetSpecs().s_Width, gBuffer->GetSpecs().s_Height));
        }

        gBuffer->Unbind();

        // lighting pass
        m_SceneFBO->Activate();
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        
        lightingPass->Activate();
        for (unsigned i = 0; i < 6; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, gTextures[i]->m_ID);
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
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_SceneFBO->GetID()); // write to scene FBO
        glBlitFramebuffer(0, 0, gBuffer->GetSpecs().s_Width, gBuffer->GetSpecs().s_Height, 
                          0, 0, m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height,
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, m_SceneFBO->GetID());
        
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
                cube->Draw(flatShader->m_ID, m_Camera.View(), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height));
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

        m_SceneFBO->Unbind();
        
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

        for (unsigned i = 0; i < static_cast<unsigned>(currLocalLights); ++i)
        {
            localLightData->GetData().pos = localLights[i].pos;
            localLightData->GetData().color = localLights[i].color;
            localLightData->GetData().options = localLights[i].options;
            localLightData->SetData();

            sphere->Update(0.0f, glm::vec3(localLights[i].pos.w * localLights[i].options.y), glm::vec3(localLights[i].pos));

            sphere->Draw(localLight->m_ID, m_Camera.View(), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height));
        }

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        for (unsigned i = 0; i < static_cast<unsigned>(currLocalLights); ++i)
        {
            localLightData->GetData().pos = localLights[i].pos;
            localLightData->GetData().color = localLights[i].color;
            localLightData->SetData();

            sphere->Update(0.0f, glm::vec3(1.0f), glm::vec3(localLights[i].pos));
            sphere->Draw(flatShader->m_ID, m_Camera.View(), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height));

            if (m_DisplayDebugRanges)
            {
                sphere->Update(0.0f, glm::vec3(localLights[i].pos.w * localLights[i].options.y), glm::vec3(localLights[i].pos));
                sphere->Draw(flatShader->m_ID, m_Camera.View(), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height), {}, GL_LINES);
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

        matrixData->GetData().view = glm::mat4(glm::mat3(m_Camera.View()));
        matrixData->SetData();

       //skybox->Update(0.0f, glm::vec3(50.0f), (m_Camera.cameraPos + skybox->getModelCentroid()) / glm::vec3(50.0f));
       skybox->Draw(skyboxShader->m_ID, glm::mat4(glm::mat3(m_Camera.View())), m_Camera.Perspective(m_SceneFBO->GetSpecs().s_Width, m_SceneFBO->GetSpecs().s_Height), skyboxTextures, GL_TRIANGLES);
       
       glDepthFunc(GL_LESS);
    }
}
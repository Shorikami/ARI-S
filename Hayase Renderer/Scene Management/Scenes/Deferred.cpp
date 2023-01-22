#include "Deferred.h"

#include <glm/vec3.hpp>
#include <glad/glad.h>

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

#include "stb/stb_image.h"

#include "../../IO/Mouse.h"

#define BUF_SIZE 512
char m_ObjPath[BUF_SIZE] = "Materials/Models/g0.obj";

int currLights = 4;
int currLocalLights = NUM_LIGHTS;

float sphereLineRad = 3.0f;
bool stopRotation = true;

// Default camera vectors
const glm::vec4 O = glm::vec4(0.0f, 1.5f, 0.0f, 1.0f),
EY = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
EZ = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

namespace Hayase
{
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    Deferred::~Deferred()
    {

    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    Deferred::Deferred(int windowWidth, int windowHeight)
        : Scene(windowWidth, windowHeight)
        , angleOfRotation(0.0f)
        , m_Camera(glm::vec3(-6.0f, 1.0f, 0.0f))
        //, m_CS250_Camera(O + 10.0f * EZ, -EZ, EY, 45.0f,
        //    (float)windowWidth / (float)windowHeight,
        //    0.1f, 1000.0f)
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

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    void Deferred::CleanUp()
    {
        sphereLine->Cleanup();

        delete geometryPass;
        delete lightingPass;
        delete localLight;
        delete flatShader;
        delete skyboxShader;

        for (unsigned i = 0; i < 4; ++i)
        {
            delete loadedObj[i];
        }

        for (unsigned i = 0; i < 3; ++i)
        {
            delete quadPlane[i];
        }

        for (unsigned i = 0; i < MAX_LIGHTS; ++i)
        {
            sphere[i]->Cleanup();
        }
    }

    float RandomNum(float min, float max)
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
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
        textures.push_back(std::make_pair(new Texture("Materials/Models/BA/Shiroko/Texture2D/Shiroko_Original_Weapon.png"), "diffTex"));
        textures.push_back(std::make_pair(new Texture("Materials/Models/BA/Shiroko/Texture2D/Shiroko_Original_Weapon.png"), "specTex"));

        groundTextures.push_back(std::make_pair(new Texture("Materials/Textures/metal_roof_diff_512x512.png"), "diffTex"));
        groundTextures.push_back(std::make_pair(new Texture("Materials/Textures/metal_roof_spec_512x512.png"), "specTex"));

        // gBuffer textures (position, normals, UVs, albedo (diffuse), specular, depth)
        for (unsigned i = 0; i < 5; ++i)
        {
            gTextures.push_back(new Texture(WindowInfo::windowWidth, WindowInfo::windowHeight, GL_RGBA16F, GL_RGBA, nullptr,
                GL_NEAREST, GL_CLAMP_TO_EDGE));
        }
        gTextures.push_back(new Texture(WindowInfo::windowWidth, WindowInfo::windowHeight, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, nullptr,
            GL_NEAREST, GL_REPEAT, GL_FLOAT));

        //for (unsigned i = 0; i < 6; ++i)
        //{
        //    gTextures.push_back(new Texture(WindowInfo::windowWidth, WindowInfo::windowHeight, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, nullptr,
        //        GL_NEAREST, GL_CLAMP_TO_EDGE, GL_FLOAT));
        //}
        
        // gBuffer FBO
        gBuffer = new Framebuffer(Framebuffer::CreateFBO({ 
            gTextures[0]->ID, gTextures[1]->ID, gTextures[2]->ID, gTextures[3]->ID, gTextures[4]->ID }, gTextures[5]->ID));

        // skybox textures
        {
            std::string common = "Materials/Textures/skybox/skybox_";
        
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
            loadedObj[i] = new Model("Materials/Models/BA/Shiroko/Mesh/Shiroko_Original_Weapon.obj", positions[i],
                glm::vec3(10.0f), 0.0f, Model::RotationAxis::yAxis, textures);
            loadedObj[i]->Name(std::string("White Fang ") + std::to_string(i + 1));
        }

        for (unsigned i = 0; i < 3; ++i)
        {
            quadPlane[i] = new Model("Materials/Models/Plane.obj", quadPositions[i].first, 
                glm::vec3(20.0f), 0.0f, quadPositions[i].second, groundTextures);
            quadPlane[i]->Name(std::string("Ground ") + std::to_string(i + 1));
        }

        quadPlane[1]->Rotate(glm::pi<float>() / 2.0f);
        quadPlane[2]->Rotate(glm::pi<float>() / 2.0f);
        quadPlane[2]->InvertAxis(true);

        skybox = new Mesh();
        skybox->initData();
        reader.ReadOBJFile("Materials/Models/cube.obj", skybox);

        sphereLine = Mesh::CreateLine(1.0f, 30);

        sphere[0] = Mesh::CreateSphere(0.08f, 8);

        for (unsigned i = 1; i < MAX_LIGHTS; ++i)
        {
            sphere[i] = sphere[0];
        }

        skybox->GenerateBuffers();

        sphereLine->GenerateBuffers(false);

        for (unsigned i = 0; i < MAX_LIGHTS; ++i)
        {
            sphere[i]->GenerateBuffers();
        }

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
        {
            for (unsigned i = 0; i < MAX_LIGHTS; ++i)
            {
                float xPos = RandomNum(-4.0f, 10.0f);
                float yPos = RandomNum(-1.f, 1.f);
                float zPos = RandomNum(-4.0f, 10.0f);
                float range = RandomNum(1.0f, 10.0f);

                float intensity = RandomNum(1.0f, 10.0f);
                float cutoff = RandomNum(0.05f, 0.1f);

                float r = RandomNum(0.0f, 1.0f);
                float g = RandomNum(0.0f, 1.0f);
                float b = RandomNum(0.0f, 1.0f);


                localLights[i].pos = glm::vec4(xPos, yPos, zPos, range);
                localLights[i].color = glm::vec4(r, g, b, 1.0f);

                float maxRange = range * (sqrtf(intensity / cutoff) - 1.0f);

                //localLights[i].options = glm::vec3(intensity, 0.01f, maxRange);
                localLights[i].options = glm::vec4(intensity, cutoff, maxRange, 1.0f);
            }
        }

        localLightData->SetData();

        matrixData->GetData().nearFar = glm::vec2(0.1f, 20.0f);
        matrixData->SetData();

        return 0;
    }

    int Deferred::preRender(float frame)
    {
        Lights& lightUBO = lightData->GetData();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Text("FPS: %.3f", frame);

        if (ImGui::BeginMainMenuBar())
        {
            
            ImGui::Text("Camera X: %.2f", m_Camera.cameraPos.x);
            ImGui::Text("Camera Y: %.2f", m_Camera.cameraPos.y);
            ImGui::Text("Camera Z: %.2f", m_Camera.cameraPos.z);

            if (ImGui::BeginMenu("Scene Settings"))
            {
                ImGui::Checkbox("Display Local Light Ranges", &m_DisplayDebugRanges);
                ImGui::Checkbox("Display Local Lights", &m_DisplayLocalLights);
                ImGui::Checkbox("Display Skybox", &m_DisplaySkybox);
                ImGui::Separator();

                ImGui::Text("Attenuation Calculation");
                {
                    static int attenCalc = 0;
                    attenCalc = m_AttenuationCalc;
                    ImGui::RadioButton("Basic", &attenCalc, 0);
                    ImGui::RadioButton("Advanced", &attenCalc, 1);
                    m_AttenuationCalc = attenCalc;
                }
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

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Models"))
            {
                for (unsigned i = 0; i < 4; ++i)
                {
                    ImGui::PushID(i);

                    ImGui::Text(loadedObj[i]->Name().c_str());
                    ImGui::Separator();

                    ImGui::SliderFloat3("Position", glm::value_ptr(loadedObj[i]->Translation()), -20.0f, 20.0f);
                    ImGui::Checkbox("Invert Axis?", &loadedObj[i]->AxisInverted());

                    static int rotAxis = 0;
                    rotAxis = static_cast<int>(loadedObj[i]->Rotation());
                    ImGui::RadioButton("X Axis", &rotAxis, 0);
                    ImGui::RadioButton("Y Axis", &rotAxis, 1);
                    ImGui::RadioButton("Z Axis", &rotAxis, 2);
                    loadedObj[i]->Rotation() = static_cast<Model::RotationAxis>(rotAxis);

                    ImGui::SliderFloat("Angle", &loadedObj[i]->Angle(), 0.0f, glm::pi<float>());
                    ImGui::SliderFloat3("Scale", glm::value_ptr(loadedObj[i]->Scale()), 0.1f, 20.0f);

                    ImGui::PopID();
                }

                for (unsigned i = 0; i < 3; ++i)
                {
                    ImGui::PushID(i + 4);

                    ImGui::Text(quadPlane[i]->Name().c_str());
                    ImGui::Separator();

                    ImGui::SliderFloat3("Position", glm::value_ptr(quadPlane[i]->Translation()), -20.0f, 20.0f);
                    ImGui::Checkbox("Invert Axis?", &quadPlane[i]->AxisInverted());

                    static int rotAxis = 0;
                    rotAxis = static_cast<int>(quadPlane[i]->Rotation());
                    ImGui::RadioButton("X Axis", &rotAxis, 0);
                    ImGui::RadioButton("Y Axis", &rotAxis, 1);
                    ImGui::RadioButton("Z Axis", &rotAxis, 2);
                    quadPlane[i]->Rotation() = static_cast<Model::RotationAxis>(rotAxis);

                    ImGui::SliderFloat("Angle", &quadPlane[i]->Angle(), 0.0f, glm::pi<float>());
                    ImGui::SliderFloat3("Scale", glm::value_ptr(quadPlane[i]->Scale()), 0.1f, 20.0f);

                    ImGui::PopID();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Local Lights"))
            {
                ImGui::SliderInt("No. of Local Lights", &currLocalLights, 1, MAX_LIGHTS);
                ImGui::Separator();

                for (unsigned i = 0; i < currLocalLights; ++i)
                {
                    ImGui::PushID(i);

                    ImGui::SliderFloat3("Light Position", glm::value_ptr(localLights[i].pos), -20.0f, 20.0f);
                    ImGui::SliderFloat("Light Range", &localLights[i].pos.w, 1.0f, 50.0f);
                    ImGui::Separator();
                    ImGui::ColorEdit4("Light Color", glm::value_ptr(localLights[i].color));
                    ImGui::Separator();
                    ImGui::SliderFloat("Intensity", &localLights[i].options.x, 0.0f, 10.0f);
                    ImGui::SliderFloat("Cutoff", &localLights[i].options.y, 0.05f, 0.1f);
                    ImGui::Separator();
                    ImGui::Text("Maximum Range: %f", localLights[i].options.z);

                    ImGui::PopID();
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGui::Render();

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

        // gBuffer pass
        gBuffer->Bind();
        glViewport(0, 0, _windowWidth, _windowHeight);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        geometryPass->Activate();

        // hard-coded :)
        for (unsigned i = 0; i < 4; ++i)
        {
            loadedObj[i]->Update();
            loadedObj[i]->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective());
        }

        for (unsigned i = 0; i < 3; ++i)
        {
            quadPlane[i]->Update();
            quadPlane[i]->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective());
        }

        Framebuffer::Unbind();

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
        
        RenderQuad();

        // Local light "pass" (it's being forward rendered)
        
        // copy depth information from the gBuffer to the default framebuffer (for the skybox, so that it doesn't overlap the FSQ)
        // (also do it for the local lights so that they're not overlapped by the skybox)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->GetID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        glBlitFramebuffer(0, 0, _windowWidth, _windowHeight, 0, 0, _windowWidth, _windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
    int Deferred::postRender()
    {
        //sphereLine->Update(angleOfRotation, glm::vec3(sphereLineRad));
        //sphereLine->Draw(lineShader->m_ID, m_Camera.view(), m_Camera.perspective(), {}, GL_LINES);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (!stopRotation)
        {
            angleOfRotation += 0.0006f;
        }
        return 0;
    }

    void Deferred::RenderQuad()
    {
        if (quadVAO == 0)
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
        }
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
        localLight->SetInt("attCalc", m_AttenuationCalc);

        glUseProgram(0);

        for (unsigned i = 0; i < currLocalLights; ++i)
        {
            localLights[i].options.z = localLights[i].pos.w * (sqrtf(localLights[i].options.x / 0.01f) - 1.0f);

            localLightData->GetData().pos = localLights[i].pos;
            localLightData->GetData().color = localLights[i].color;
            localLightData->GetData().options = localLights[i].options;
            localLightData->SetData();

            sphere[i]->Update(0.0f, glm::vec3(localLights[i].options.z), glm::vec3(localLights[i].pos));

            sphere[i]->Draw(localLight->m_ID, m_Camera.view(), m_Camera.perspective());

            if (m_DisplayDebugRanges)
            {
                sphere[i]->Draw(flatShader->m_ID, m_Camera.view(), m_Camera.perspective(), {}, GL_LINES);
            }
        }

        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        //glEnable(GL_DEPTH_TEST);
        //glDisable(GL_BLEND);

        for (unsigned i = 0; i < currLocalLights; ++i)
        {
            localLightData->GetData().pos = localLights[i].pos;
            localLightData->GetData().color = localLights[i].color;
            localLightData->SetData();

            sphere[i]->Update(0.0f, glm::vec3(1.0f), glm::vec3(localLights[i].pos));
            sphere[i]->Draw(flatShader->m_ID, m_Camera.view(), m_Camera.perspective());
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

    void Deferred::ProcessInput(GLFWwindow* win, float dt)
    {
        if (!ImGui::IsItemActive() && ImGui::TempInputIsActive(ImGui::GetActiveID()))
        {
            if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
                m_Camera.UpdateCameraPos(CameraDirection::FORWARD, dt);
            if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
                m_Camera.UpdateCameraPos(CameraDirection::BACKWARDS, dt);
            if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
                m_Camera.UpdateCameraPos(CameraDirection::LEFT, dt);
            if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
                m_Camera.UpdateCameraPos(CameraDirection::RIGHT, dt);
        }

        double dx = Mouse::GetDX(); double dy = Mouse::GetDY();
        if (dx != 0 || dy != 0)
        {
            float sens = 1.f;
            m_Camera.UpdateCameraDir(dx * sens, dy * sens);
        }

        if (glfwGetKey(win, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
            m_Camera.rotateCamera = !m_Camera.rotateCamera;
    }

    void Deferred::ProcessMouse(float x, float y)
    {
        m_Camera.UpdateCameraDir(x, y);
    }
}
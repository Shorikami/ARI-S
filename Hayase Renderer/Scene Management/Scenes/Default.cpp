#include "Default.h"

#include <glm/vec3.hpp>
#include <glad/glad.h>

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

#include "stb/stb_image.h"

#include "../../IO/Mouse.h"

#define NUM_LIGHTS 1
#define MAX_LIGHTS 16

#define BUF_SIZE 512
char m_ObjPath[BUF_SIZE] = "Materials/Models/g0.obj";

int currLights = NUM_LIGHTS;
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
    Default::~Default()
    {

    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    Default::Default(int windowWidth, int windowHeight)
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
    void Default::initMembers()
    {
        //shaderID = lineShaderID = 0;
        angleOfRotation = 0.0f;

        matrixData = new UniformBuffer<World>(0);
        lightData = new UniformBuffer<Lights>(1);
    }
#pragma clang diagnostic pop

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    void Default::CleanUp()
    {
        loadedObj->Cleanup();
        quadPlane->Cleanup();

        sphereLine->Cleanup();

        delete lineShader;
        delete geometryPass;
        delete lightingPass;
        delete localLight;
        delete skyboxShader;

        // delete sphere shader?

        for (unsigned i = 0; i < 16; ++i)
            sphere[i]->Cleanup();

        // delete skybox texture?
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int Default::Init()
    {
        // Create and compile our GLSL program from the shaders
        lineShader = new Shader(false, "LineShader.vert", "LineShader.frag");

        geometryPass = new Shader(false, "Deferred/GeometryPass.vert", "Deferred/GeometryPass.frag");
        lightingPass = new Shader(false, "Deferred/LightingPass.vert", "Deferred/LightingPass.frag");
        localLight = new Shader(false, "Deferred/LocalLight.vert", "Deferred/LocalLight.frag");

        skyboxShader = new Shader(false, "Reflections/Skybox.vert", "Reflections/Skybox.frag");

        // Object textures
        textures.push_back(std::make_pair(new Texture("Materials/Models/BA/Shiroko/Texture2D/Shiroko_Original_Weapon.png"), "diffTex"));
        textures.push_back(std::make_pair(new Texture("Materials/Textures/metal_roof_spec_512x512.png"), "specTex"));

        // gBuffer textures (position, normals, UVs, albedo (diffuse), specular, depth)
        for (unsigned i = 0; i < 6; ++i)
        {
            gTextures.push_back(new Texture(WindowInfo::windowWidth, WindowInfo::windowHeight, GL_RGBA16F, GL_RGBA, nullptr,
                GL_NEAREST, GL_CLAMP_TO_EDGE));
        }
        
        // gBuffer FBO
        gBuffer = new Framebuffer(Framebuffer::CreateFBO({ 
            gTextures[0]->ID, gTextures[1]->ID, gTextures[2]->ID, gTextures[3]->ID, gTextures[4]->ID, gTextures[5]->ID }));

        //glGenFramebuffers(1, &gBuf);
        //glBindFramebuffer(GL_FRAMEBUFFER, gBuf);
        //
        //// color buffers
        //for (unsigned i = 0; i < 6; ++i)
        //{
        //    glGenTextures(1, &gTex[i]);
        //    glBindTexture(GL_TEXTURE_2D, gTex[i]);
        //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _windowWidth, _windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, gTex[i], 0);
        //}
        //
        //unsigned attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
        //GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
        //glDrawBuffers(6, attachments);
        //
        //// depth buffer
        //glGenRenderbuffers(1, &rbo);
        //glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _windowWidth, _windowHeight);
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
        //
        //if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        //    std::cout << "Framebuffer not complete!" << std::endl;
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // skybox textures
        //{
        //    std::string common = "Materials/Textures/skybox/skybox_";
        //
        //    skyboxTextures.push_back(std::make_pair(new Texture(common + "right.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[0]"));
        //    skyboxTextures.push_back(std::make_pair(new Texture(common + "left.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[1]"));
        //    skyboxTextures.push_back(std::make_pair(new Texture(common + "top.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[2]"));
        //    skyboxTextures.push_back(std::make_pair(new Texture(common + "bottom.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[3]"));
        //    skyboxTextures.push_back(std::make_pair(new Texture(common + "front.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[4]"));
        //    skyboxTextures.push_back(std::make_pair(new Texture(common + "back.jpg", GL_NEAREST, GL_CLAMP_TO_EDGE), "cubeTexture[5]"));
        //}

        loadedObj = new Mesh("Materials/Models/BA/Shiroko/Mesh/Shiroko_Original_Weapon.obj");
        //loadedObj->initData();
        //reader.ReadOBJFile(m_ObjPath, loadedObj);

        quadPlane = new Mesh();
        quadPlane->initData();
        reader.ReadOBJFile("Materials/Models/quad.obj", quadPlane, false);

        skybox = new Mesh();
        skybox->initData();
        reader.ReadOBJFile("Materials/Models/cube.obj", skybox);

        sphereLine = Mesh::CreateLine(1.0f, 30);

        sphere[0] = Mesh::CreateSphere(0.08f, 8);

        for (unsigned i = 1; i < MAX_LIGHTS; ++i)
        {
            sphere[i] = sphere[0];
        }

        loadedObj->GenerateBuffers();
        quadPlane->GenerateBuffers();
        skybox->GenerateBuffers();

        sphereLine->GenerateBuffers(false);

        for (unsigned i = 0; i < MAX_LIGHTS; ++i)
        {
            sphere[i]->GenerateBuffers();
        }

        Lights& lightUBO = lightData->GetData();

        for (unsigned i = 0; i < MAX_LIGHTS; ++i)
        {
            lightUBO.lightColor[i] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

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

        matrixData->GetData().nearFar = glm::vec2(0.1f, 20.0f);
        matrixData->SetData();

        return 0;
    }

    int Default::preRender()
    {
        Lights& lightUBO = lightData->GetData();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Tools");

        if (ImGui::CollapsingHeader("Model"))
        {
            if (ImGui::InputText("Object Path", m_ObjPath, BUF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                loadedObj->Cleanup();
                loadedObj->initData();
                reader.ReadOBJFile(m_ObjPath, loadedObj);
                loadedObj->GenerateBuffers();
            }
        }

        if (ImGui::CollapsingHeader("Normal Display Options"))
        {
            ImGui::Checkbox(("Toggle Vertex Normals"), &m_DisplayVertNorms);
            ImGui::Checkbox(("Toggle Face Normals"), &m_DisplayFaceNorms);
        }

        if (ImGui::CollapsingHeader("Background Color"))
        {
            ImGui::ColorPicker3("BG Color", (float*)(&m_BGColor));
        }

        if (ImGui::CollapsingHeader("Light Properties"))
        {
            ImGui::Checkbox(("Disable Rotation"), &stopRotation);
            ImGui::SliderInt("No. of Lights", &currLights, 1, 16);
            ImGui::SliderFloat("Orbit Radius", &sphereLineRad, 0.1f, 10.0f);

            ImGui::Text("CPU or GPU?");
            const char* items[] = { "CPU", "GPU" };
            static const char* select = NULL;
            if (ImGui::BeginCombo("##combo", select))
            {
                for (unsigned n = 0; n < IM_ARRAYSIZE(items); ++n)
                {
                    bool is_selected = (select == items[n]);
                    if (ImGui::Selectable(items[n], is_selected))
                    {
                        select = items[n];
                        lightUBO.modes.x = select == "CPU" ? 0 : 1;
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Text("UV Calculation");
            const char* types[] = { "Planar", "Cylindrical", "Spherical", "Cubical" };
            static const char* uvSelect = NULL;
            static bool calculatedOnce = false;
            static int prev = -1;

            if (ImGui::BeginCombo("##combo2", uvSelect))
            {
                for (unsigned n = 0; n < IM_ARRAYSIZE(types); ++n)
                {
                    bool is_selected = (uvSelect == types[n]);
                    if (ImGui::Selectable(types[n], is_selected))
                    {

                        uvSelect = types[n];
                        lightUBO.modes.z = n;

                        if (prev != lightUBO.modes.z)
                        {
                            prev = lightUBO.modes.z;
                            calculatedOnce = false;
                        }

                        if (!calculatedOnce)
                        {
                            loadedObj->calcUVs(static_cast<Mesh::UVType>(n));
                            loadedObj->Cleanup();
                            loadedObj->GenerateBuffers();
                            calculatedOnce = true;
                        }
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Text("Texture Entity");
            {
                static int k = 0;
                k = lightUBO.modes.y;
                ImGui::RadioButton("Position", &k, 0);
                ImGui::RadioButton("Normals", &k, 1);
                lightUBO.modes.y = k;
            }

            ImGui::Text("Global Values");

            ImGui::ColorEdit4("Fog Color", glm::value_ptr(lightUBO.fogColor));
            ImGui::SliderFloat("Near", &matrixData->GetData().nearFar.x, 0.1f, 100.0f);
            ImGui::SliderFloat("Far", &matrixData->GetData().nearFar.y, 1.0f, 100.0f);

            ImGui::ColorEdit4("Global Ambient", glm::value_ptr(lightUBO.globalAmbient));

            ImGui::SliderFloat3("Light Coefficients", glm::value_ptr(lightUBO.coefficients), 0.0f, 1.0f);
            ImGui::SliderFloat("C1", &lightUBO.attenuation.x, 0.0f, 1.0f);
            ImGui::SliderFloat("C2", &lightUBO.attenuation.y, 0.0f, 1.0f);
            ImGui::SliderFloat("C3", &lightUBO.attenuation.z, 0.0f, 1.0f);

            for (unsigned i = 0; i < currLights; ++i)
            {
                ImGui::PushID(i);
                std::string id = std::string(1, (char)('0' + i));
                if (ImGui::TreeNode(id.c_str(), "Light %i", i))
                {
                    ImGui::ColorEdit4("Ambient Color", (float*)(glm::value_ptr(lightUBO.ambient[i])));
                    ImGui::ColorEdit4("Diffuse Color", (float*)(glm::value_ptr(lightUBO.diffuse[i])));
                    ImGui::ColorEdit4("Specular Color", (float*)(glm::value_ptr(lightUBO.specular[i])));

                    ImGui::Text("Light Type");
                    {
                        static int temp = 0;
                        temp = (int)lightUBO.lightInfo[i][3];
                        ImGui::RadioButton("Point", &temp, 0);
                        ImGui::RadioButton("Directional", &temp, 1);
                        ImGui::RadioButton("Spotlight", &temp, 2);
                        lightUBO.lightInfo[i][3] = (float)temp;
                    }

                    if ((int)lightUBO.lightInfo[i][3] == 2)
                    {
                        ImGui::Text("Phi is the OUTER angle");
                        ImGui::Text("Theta is the INNER angle");

                        ImGui::Columns(3);
                        ImGui::SliderFloat("Falloff", &lightUBO.lightInfo[i].z, 0.1f, 10.0f);
                        ImGui::NextColumn();
                        ImGui::SliderFloat("Theta", &lightUBO.lightInfo[i].y, 0.01f, 90.0f);
                        ImGui::NextColumn();
                        ImGui::SliderFloat("Phi", &lightUBO.lightInfo[i].x, 0.01f, 90.0f);
                        ImGui::Columns(1);
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
        ImGui::End();
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
    int Default::Render()
    {
        matrixData->GetData().proj = m_Camera.perspective();
        matrixData->GetData().view = m_Camera.view();
        matrixData->SetData();

        glClearColor(m_BGColor.x, m_BGColor.y, m_BGColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gBuffer->Bind();

        //glBindFramebuffer(GL_FRAMEBUFFER, gBuf);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        geometryPass->Activate();

        //std::vector<std::pair<Texture*, std::string>>()

        loadedObj->Update(0.0f, glm::vec3(10.0f));
        loadedObj->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), textures, GL_TRIANGLES,
            m_DisplayVertNorms, m_DisplayFaceNorms, false);

        loadedObj->Update(0.0f, glm::vec3(10.0f), glm::vec3(8.0f, 0.0f, 0.0f));
        loadedObj->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), textures, GL_TRIANGLES,
            m_DisplayVertNorms, m_DisplayFaceNorms, false);

        loadedObj->Update(0.0f, glm::vec3(10.0f), glm::vec3(0.0f, 0.0f, 8.0f));
        loadedObj->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), textures, GL_TRIANGLES,
            m_DisplayVertNorms, m_DisplayFaceNorms, false);

        loadedObj->Update(0.0f, glm::vec3(10.0f), glm::vec3(8.0f, 0.0f, 8.0f));
        loadedObj->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), textures, GL_TRIANGLES,
            m_DisplayVertNorms, m_DisplayFaceNorms, false);

        Framebuffer::Unbind();
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
            //glBindTexture(GL_TEXTURE_2D, gTex[i]);
            glBindTexture(GL_TEXTURE_2D, gTextures[i]->ID);
        }

        lightingPass->SetVec3("viewPos", m_Camera.cameraPos);
        
        RenderQuad();

        // TODO: Local light pass

        return 0;
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int Default::postRender()
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

    void Default::RenderQuad()
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

    void Default::RenderActualScene()
    {
        //RenderLights(m_Camera.view());
        //
        //loadedObj->Update(0.0f);
        //loadedObj->Draw(envMapShader->m_ID, m_Camera.view(), m_Camera.perspective(), cameraTextures, GL_TRIANGLES,
        //    m_DisplayVertNorms, m_DisplayFaceNorms, false);
        //
        ////quadPlane->Update(3.14f / 2, glm::vec3(10.0f), glm::vec3(-0.5f, -0.5f, -0.2f), glm::vec3(-1.0f, 0.0f, 0.0f));
        ////quadPlane->Draw(currShader, m_Camera.view(), m_Camera.perspective(), {}, GL_TRIANGLES);
        //
        //RenderSkybox(m_Camera.view());
    }

    void Default::RenderLights(glm::mat4 view)
    {
        //Lights& lightUBO = lightData->GetData();
        //for (unsigned i = 0; i < currLights; ++i)
        //{
        //    // Passing in diffuse for current light
        //    glUseProgram(diffuseShader->m_ID);
        //    glm::vec3 value = glm::vec3(0.7f, 0.7f, 0.7f);
        //    glUniform3fv(glGetUniformLocation(diffuseShader->m_ID, "lightColor"), 1, glm::value_ptr(glm::vec3(lightUBO.diffuse[i])));
        //    glUseProgram(0);
        //
        //    sphere[i]->Update(angleOfRotation + (2 * 3.14f / currLights) * i, glm::vec3(1.0f), glm::vec3(0, 0, sphereLineRad));
        //    sphere[i]->Draw(diffuseShader->m_ID, view, m_Camera.perspective(), {}, GL_TRIANGLES, m_DisplayVertNorms, m_DisplayFaceNorms);
        //
        //    glm::mat4 modelMat = sphere[i]->getModelMat();
        //
        //    lightUBO.lightPos[i] = glm::column(sphere[i]->getModelMat(), 3);
        //    lightUBO.lightDir[i] = glm::normalize(glm::column(loadedObj->getModelMat(), 3) - lightUBO.lightPos[i]);
        //}
        //lightUBO.numLights = currLights;
        //lightData->SetData();
    }

    void Default::RenderSkybox(glm::mat4 view)
    {
        //glDepthFunc(GL_LEQUAL);
        //
        //skybox->Update(0.0f, glm::vec3(50.0f), (m_Camera.cameraPos + skybox->getModelCentroid()) / glm::vec3(50.0f));
        //skybox->Draw(skyboxShader->m_ID, view, m_Camera.perspective(), skyboxTextures, GL_TRIANGLES);
        //
        //glDepthFunc(GL_LESS);
    }

    void Default::ProcessInput(GLFWwindow* win, float dt)
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

    void Default::ProcessMouse(float x, float y)
    {
        m_Camera.UpdateCameraDir(x, y);
    }
}
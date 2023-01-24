#include "DepthBuffer.h"

#include <glm/vec3.hpp>
#include <glad/glad.h>

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

#include "../../IO/Mouse.h"

namespace Hayase
{
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    DepthBuffer::~DepthBuffer()
    {

    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    DepthBuffer::DepthBuffer(int windowWidth, int windowHeight)
        : Scene(windowWidth, windowHeight)
        , m_Camera(glm::vec3(-6.0f, 1.0f, 0.0f))
    {
        initMembers();
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-unused-return-value"
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    void DepthBuffer::initMembers()
    {
        matrixData = new UniformBuffer<World>(0);
    }
#pragma clang diagnostic pop

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    void DepthBuffer::CleanUp()
    {
        delete depthPass;
        delete FSQShader;
        delete skyboxShader;

        for (unsigned i = 0; i < 4; ++i)
        {
            delete loadedObj[i];
        }

        for (unsigned i = 0; i < 3; ++i)
        {
            delete quadPlane[i];
        }

        skybox->Cleanup();
    }
    
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int DepthBuffer::Init()
    {
        depthPass = new Shader(false, "DepthTexture/Depth.vert", "DepthTexture/Depth.frag");
        FSQShader = new Shader(false, "DepthTexture/FSQ.vert", "DepthTexture/FSQ.frag");

        skyboxShader = new Shader(false, "Reflections/Skybox.vert", "Reflections/Skybox.frag");

        glGenFramebuffers(1, &depthMapFBO);
        // create depth texture
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _windowWidth, _windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
                glm::vec3(10.0f), 0.0f, Model::RotationAxis::yAxis, {});
            loadedObj[i]->Name(std::string("White Fang ") + std::to_string(i + 1));
        }

        for (unsigned i = 0; i < 3; ++i)
        {
            quadPlane[i] = new Model("Materials/Models/Plane.obj", quadPositions[i].first,
                glm::vec3(20.0f), 0.0f, quadPositions[i].second, {});
            quadPlane[i]->Name(std::string("Ground ") + std::to_string(i + 1));
        }

        quadPlane[1]->Rotate(glm::pi<float>() / 2.0f);
        quadPlane[2]->Rotate(glm::pi<float>() / 2.0f);
        quadPlane[2]->InvertAxis(true);

        skybox = new Mesh();
        skybox->initData();
        reader.ReadOBJFile("Materials/Models/cube.obj", skybox);

        skybox->GenerateBuffers();

        matrixData->GetData().nearFar = glm::vec2(1.0f, 7.5f);
        matrixData->SetData();

        FSQShader->Activate();
        FSQShader->SetInt("depthMap", 0);

        glEnable(GL_DEPTH_TEST);

        return 0;
    }

    int DepthBuffer::PreRender(float frame)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Text("FPS: %.3f", frame);

        if (ImGui::BeginMainMenuBar())
        {

            ImGui::Text("Camera X: %.2f", m_Camera.cameraPos.x);
            ImGui::Text("Camera Y: %.2f", m_Camera.cameraPos.y);
            ImGui::Text("Camera Z: %.2f", m_Camera.cameraPos.z);

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

            ImGui::EndMainMenuBar();
        }

        ImGui::Render();

        return 0;
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int DepthBuffer::Render()
    {
        matrixData->GetData().proj = m_Camera.perspective();
        matrixData->GetData().view = m_Camera.view();
        matrixData->SetData();

        glClearColor(m_BGColor.x, m_BGColor.y, m_BGColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // gBuffer pass
        //depthBuffer->Bind();
        glViewport(0, 0, _windowWidth, _windowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        for (unsigned i = 0; i < 4; ++i)
        {
            loadedObj[i]->Update();
            loadedObj[i]->Draw(depthPass->m_ID, m_Camera.view(), m_Camera.perspective());
        }

        for (unsigned i = 0; i < 3; ++i)
        {
            quadPlane[i]->Update();
            quadPlane[i]->Draw(depthPass->m_ID, m_Camera.view(), m_Camera.perspective());
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glViewport(0, 0, _windowWidth, _windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        FSQShader->Activate();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        RenderQuad();
        glUseProgram(0);

        return 0;
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int DepthBuffer::PostRender()
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return 0;
    }

    void DepthBuffer::RenderQuad()
    {
        if (quadVAO == 0)
        {
            float quadVertices[] = 
            {
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

    void DepthBuffer::RenderSkybox()
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

    void DepthBuffer::ProcessInput(GLFWwindow* win, float dt)
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
}
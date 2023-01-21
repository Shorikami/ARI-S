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

#define NUM_LIGHTS 32
#define MAX_LIGHTS 32

#define BUF_SIZE 512
char m_ObjPath[BUF_SIZE] = "Materials/Models/g0.obj";

int currLights = 1;
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
        localLightData = new UniformBuffer<LocalLight>(2);
    }
#pragma clang diagnostic pop

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    void Default::CleanUp()
    {
        loadedObj->Cleanup();
        quadPlane->Cleanup();

        sphereLine->Cleanup();

        delete geometryPass;
        delete lightingPass;
        delete localLight;
        delete flatShader;
        delete skyboxShader;

        // delete sphere shader?

        for (unsigned i = 0; i < MAX_LIGHTS; ++i)
            sphere[i]->Cleanup();

        // delete skybox texture?
    }

    float RandomNum(float min, float max)
    {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    int Default::Init()
    {
        flatShader = new Shader(false, "FlatShader.vert", "FlatShader.frag");

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

        quadPlane = new Mesh("Materials/Models/Plane.obj");

        //quadPlane = new Mesh();
        //quadPlane->initData();
        //reader.ReadOBJFile("Materials/Models/quad.obj", quadPlane, false);

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

        // Regular light UBO (Obsolete?)
        {
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
        }

        // Local light pass UBO
        {
            for (unsigned i = 0; i < NUM_LIGHTS; ++i)
            {
                float xPos = RandomNum(-4.0f, 10.0f);
                float yPos = RandomNum(-1.f, 1.f);
                float zPos = RandomNum(-4.0f, 10.0f);
                float range = RandomNum(1.0f, 10.0f);

                float intensity = RandomNum(1.0f, 10.0f);

                float r = RandomNum(1.0f, 1.0f);
                float g = RandomNum(1.0f, 1.0f);
                float b = RandomNum(1.0f, 1.0f);

                localLights[i].pos = glm::vec4(xPos, yPos, zPos, range);
                localLights[i].color = glm::vec4(r, g, b, 1.0f);

                float maxRange = range * (sqrtf(intensity / 0.01f) - 1.0f);

                //localLights[i].options = glm::vec3(intensity, 0.01f, maxRange);
                localLights[i].options = glm::vec3(2, 0.01f, 130);
            }
        }

        localLightData->SetData();

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

        if (ImGui::BeginMainMenuBar())
        {
            ImGui::Text("Camera X: %f", m_Camera.cameraPos.x);
            ImGui::Text("Camera Y: %f", m_Camera.cameraPos.y);
            ImGui::Text("Camera Z: %f", m_Camera.cameraPos.z);

            if (ImGui::BeginMenu("Scene Settings"))
            {
                ImGui::Checkbox("Display Local Light Ranges", &m_DisplayDebugRanges);
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
    int Default::Render()
    {
        matrixData->GetData().proj = m_Camera.perspective();
        matrixData->GetData().view = m_Camera.view();
        matrixData->SetData();

        glClearColor(m_BGColor.x, m_BGColor.y, m_BGColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // gBuffer pass
        gBuffer->Bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        geometryPass->Activate();

        loadedObj->Update(0.0f, glm::vec3(10.0f));
        loadedObj->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), textures, GL_TRIANGLES,
            false, false, false);

        loadedObj->Update(0.0f, glm::vec3(10.0f), glm::vec3(8.0f, 0.0f, 0.0f));
        loadedObj->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), textures, GL_TRIANGLES,
            false, false, false);

        loadedObj->Update(0.0f, glm::vec3(10.0f), glm::vec3(0.0f, 0.0f, 8.0f));
        loadedObj->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), textures, GL_TRIANGLES,
            false, false, false);

        loadedObj->Update(0.0f, glm::vec3(10.0f), glm::vec3(8.0f, 0.0f, 8.0f));
        loadedObj->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), textures, GL_TRIANGLES,
            false, false, false);

        quadPlane->Update(0.0f, glm::vec3(20.0f), glm::vec3(3.0f, -2.0f, 5.0f));
        quadPlane->Draw(geometryPass->m_ID, m_Camera.view(), m_Camera.perspective(), {}, GL_TRIANGLES,
            false, false, false);

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
        
        RenderQuad();

        // Local light "pass" (it's being forward rendered)
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        localLight->Activate();
        localLight->SetVec3("eyePos", m_Camera.cameraPos);
        glUseProgram(0);

        for (unsigned i = 0; i < 1; ++i)
        {
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

            sphere[i]->Update(0.0f, glm::vec3(1.0f), glm::vec3(localLights[i].pos));
            sphere[i]->Draw(flatShader->m_ID, m_Camera.view(), m_Camera.perspective());
        }

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
#ifndef SCENEFRAMEWORK_H
#define SCENEFRAMEWORK_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window.h"
#include "Memory/FrameMemory.hpp"

#define _GET_GL_ERROR   { GLenum err = glGetError(); std::cout << "[OpenGL Error] " << glewGetErrorString(err) << std::endl; }

namespace Hayase
{
    class Scene
    {

    public:
        Scene();
        Scene(int windowWidth, int windowHeight);
        virtual ~Scene();

        virtual int Init();
        virtual int Display(float frameTime = 0.0f);

        virtual int PreRender(float frame);
        virtual int Render();
        virtual int PostRender();

        virtual void ProcessInput(GLFWwindow* w, float dt);

        virtual void CleanUp();

        Framebuffer* GetSceneFBO() { return m_SceneFBO; }

    protected:
        int _windowHeight, _windowWidth;
        Framebuffer* m_SceneFBO;
    };
}

#endif

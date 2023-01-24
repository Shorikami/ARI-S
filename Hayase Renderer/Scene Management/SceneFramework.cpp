#include "SceneFramework.h"

namespace Hayase
{
    Scene::Scene() 
        : _windowWidth(100)
        , _windowHeight(100)
        , window(_windowWidth, _windowHeight, "Hayase Renderer")
    {

    }

    Scene::Scene(int windowWidth, int windowHeight)
        : _windowWidth(windowWidth)
        , _windowHeight(windowHeight)
        , window(windowWidth, windowHeight, "Hayase Renderer")
    {
    }

    Scene::~Scene()
    {
        CleanUp();
    }

    int Scene::Init()
    {
        return -1;
    }

    int Scene::PreRender(float frame)
    {
        return -1;
    }

    int Scene::Render()
    {
        return -1;
    }

    int Scene::PostRender()
    {
        return -1;
    }

    void Scene::ProcessInput(GLFWwindow* w, float dt)
    {
        return;
    }

    void Scene::CleanUp()
    {
        return;
    }

    int Scene::Display(float frameTime)
    {
        PreRender(frameTime);
        Render();
        PostRender();

        return -1;
    }
}
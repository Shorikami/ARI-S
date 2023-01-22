#include "SceneFramework.h"

namespace Hayase
{
    Scene::Scene() : _windowWidth(100), _windowHeight(100)
    {

    }

    Scene::Scene(int windowWidth, int windowHeight)
    {
        _windowHeight = windowHeight;
        _windowWidth = windowWidth;
    }

    Scene::~Scene()
    {
        CleanUp();
    }

    // Public methods

    // Init: called once when the scene is initialized
    int Scene::Init()
    {
        return -1;
    }

    // LoadAllShaders: This is the placeholder for loading the shader files
    void Scene::LoadAllShaders()
    {
        return;
    }


    // preRender : called to setup stuff prior to rendering the frame
    int Scene::preRender(float frame)
    {
        return -1;
    }

    // Render : per frame rendering of the scene
    int Scene::Render()
    {
        return -1;
    }

    // postRender : Any updates to calculate after current frame
    int Scene::postRender()
    {
        return -1;
    }

    // ProcessInput : Update according to user input
    void Scene::ProcessInput(GLFWwindow* w, float dt)
    {
        return;
    }

    // ProcessMouse : Update mouse (for the camera)
    void Scene::ProcessMouse(float x, float y)
    {
        return;
    }

    // CleanUp : clean up resources before destruction
    void Scene::CleanUp()
    {
        return;
    }

    // Display : Per-frame execution of the scene
    int Scene::Display(float frameTime)
    {
        preRender(frameTime);
        Render();
        postRender();

        return -1;
    }
}
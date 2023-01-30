#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "AppEvent.h"
#include "MouseEvent.h"
#include "KeyEvent.h"
#include "Timer.h"

namespace Hayase
{
    class Event;

    enum class CameraDirection
    {
        NONE = 0,
        FORWARD,
        BACKWARDS,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    class Camera
    {
    public:
        glm::vec3 cameraPos;

        glm::vec3 front, up, right;

        float yaw, pitch, speed, zoom;
        float n, f;

        // mouse movement-dependent
        float lastX, lastY;
        bool firstMouse;

        bool rotateCamera = false;

        Camera(glm::vec3 pos = glm::vec3(0.0f));

        void Update(DeltaTime dt);
        void OnEvent(Event& e);

        void UpdateCameraDir(double dx, double dy);
        void UpdateCameraPos(CameraDirection d, double dt);
        void UpdateCameraZoom(double dy);

        __inline glm::mat4 View() { return glm::lookAt(cameraPos, cameraPos + front, up); };
        //__inline glm::mat4 Perspective()
        //{
        //    WindowBase& w = Application::Get().GetWindow();
        //
        //    return glm::perspective(glm::radians(zoom),
        //        static_cast<float>(w.GetWidth()) / static_cast<float>(w.GetHeight()),
        //        n, f);
        //}

        __inline glm::mat4 Perspective(uint32_t w, uint32_t h)
        {
            return glm::perspective(glm::radians(zoom),
                static_cast<float>(w) / static_cast<float>(h),
                n, f);
        }

        __inline float GetZoom() { return zoom; };

    private:
        void UpdateCameraVectors();

        bool OnKeyPressed(KeyPressedEvent& e);

        bool OnMouseMoved(MouseMovedEvent& e);
        bool OnMouseScrolled(MouseScrolledEvent& e);
    };
}


#endif
#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "Global.hpp"

namespace Hayase
{
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

        bool rotateCamera = false;

        Camera(glm::vec3 pos = glm::vec3(0.0f));

        void UpdateCameraDir(double dx, double dy);
        void UpdateCameraPos(CameraDirection d, double dt);
        void UpdateCameraZoom(double dy);

        __inline glm::mat4 view() { return glm::lookAt(cameraPos, cameraPos + front, up); };
        __inline glm::mat4 perspective()
        {
            return glm::perspective(glm::radians(zoom),
                static_cast<float>(WindowInfo::windowWidth) / static_cast<float>(WindowInfo::windowHeight),
                n, f);
        }

        __inline float GetZoom() { return zoom; };

    private:
        void UpdateCameraVectors();
    };

    class CS250_Camera
    {
    public:
        CS250_Camera(void);
        CS250_Camera(const glm::vec4& E, const glm::vec4& look, const glm::vec4& rel,
            float fov, float aspect, float N, float F);
        glm::vec4 eye() const { return eye_point; }
        glm::vec4 right() const { return right_vector; }
        glm::vec4 up() const { return up_vector; }
        glm::vec4 back() const { return back_vector; }
        glm::vec4 viewport() const;
        float nearDist() const { return near_distance; }
        float farDist() const { return far_distance; }
        CS250_Camera& zoom(float factor);
        CS250_Camera& forward(float distance);
        CS250_Camera& yaw(float angle);
        CS250_Camera& pitch(float angle);
        CS250_Camera& roll(float angle);

        glm::mat4 rotate(float deg, const glm::vec4& v);
        glm::mat4 affine(const glm::vec4& Lx, const glm::vec4& Ly, const glm::vec4& Lz, const glm::vec4& C);
        glm::mat4 affineInv(const glm::mat4& A);

        glm::mat4 model();
        glm::mat4 view();
        glm::mat4 perspective();
    private:
        glm::vec4 eye_point;
        glm::vec4 right_vector, up_vector, back_vector;
        float vp_width, vp_height, vp_distance,
            near_distance, far_distance;
    };
}


#endif
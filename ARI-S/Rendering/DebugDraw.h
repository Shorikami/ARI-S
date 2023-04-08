#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H

#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <vec3.hpp>
#include <detail/type_mat4x4.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_access.hpp>

#include <debug_draw.hpp>

#include "Cameras/EditorCamera.h"

namespace ARIS
{
    class DebugInterface : public dd::RenderInterface
    {
    public:
        void drawPointList(const dd::DrawVertex* points, int count, bool depthEnabled) override;
        void drawLineList(const dd::DrawVertex* lines, int count, bool depthEnabled) override;
        ~DebugInterface() override;

        DebugInterface();

        void SetupShaders();
        void SetupVertexBuffers();

        static void CompileShader(const GLuint s);
        static void LinkProgram(const GLuint p);

        glm::mat4 m_MatrixMVP;
    private:
        GLuint m_LineProgram;
        GLint m_LineProgramMatrixLoc;

        GLuint m_LineVAO;
        GLuint m_LineVBO;

        static const char* m_LineVertexShader;
        static const char* m_LineFragmentShader;
    };

    class DebugWrapper
    {
    public:
        static DebugWrapper& GetInstance()
        {
            static DebugWrapper i;
            return i;
        }

        void Initialize();
        void Destroy();

        void Update(EditorCamera& c);
        void Render();

    private:
        DebugInterface m_Interface;
    };
}

#endif
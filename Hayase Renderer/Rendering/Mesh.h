#ifndef MESH_H
#define MESH_H

#include <glm.hpp>

#include "Memory/VertexMemory.hpp"
#include "Memory/UniformMemory.hpp"
#include "Texture.h"

namespace Hayase
{
    class Mesh
    {
    public:
        friend class OBJReader;

        enum class DisplayType
        {
            VERTEX = 0,
            FACE
        };

        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 uv;
            glm::vec3 color;

            bool operator==(const Vertex& other) const
            {
                return position == other.position &&
                    color == other.color &&
                    normal == other.normal &&
                    uv == other.uv;
            }
        };

        Mesh();
        Mesh(std::string path, std::string mtlPath = std::string());

        static Mesh* CreateSphere(float rad, unsigned divisions);
        static Mesh* CreateLine(float rad, unsigned divisions);

        // Get attribute values buffer

        GLfloat* getVertexBuffer();             // attribute 0
        glm::vec3* getVertexBufferVec3();

        GLfloat* getVertexNormals();            // attribute 1
        glm::vec3* getVertexNormalsVec3();

        GLfloat* getVertexUVs();                // attribute 2
        glm::vec2* getVertexUVsVec2();

        GLfloat* getDisplayNormals(DisplayType t);
        glm::vec3* getDisplayNormalsVec3(DisplayType t);
        unsigned int getDisplayCount(DisplayType t);
        GLuint* getDisplayIndexBuffer(DisplayType t);
        unsigned int getDisplayIndexBufferSize(DisplayType t);

        unsigned int getVertexBufferSize();
        unsigned int getVertexCount();
        unsigned int getVertexNormalCount();
        unsigned int getVertexUVCount();


        // Get vertex index buffer
        GLuint* getIndexBuffer();
        unsigned int getIndexBufferSize();

        unsigned int getTriangleCount();

        glm::vec3  getModelScale();
        glm::vec3  getModelCentroid();
        glm::vec3  getCentroidVector(glm::vec3 vVertex);

        void ScaleNegOneToOne();

        GLfloat& getNormalLength();
        void setNormalLength(GLfloat nLength);

        // initialize the data members
        void initData();

        // Generate buffers (after generating data)
        void GenerateBuffers(bool hasNormals = true);

        __inline glm::mat4 getModelMat() { return modelMat; };

        // Update object
        void Update(float angle, glm::vec3 scale = glm::vec3(1.0f), glm::vec3 centroid = glm::vec3(0.0f),
            glm::vec3 angleOfRot = glm::vec3(0.0f, 1.0f, 0.0f));

        // Draw object
        void Draw(GLuint id, glm::mat4 view, glm::mat4 proj, std::vector<std::pair<Texture*, std::string>> textures = {}, GLenum type = GL_TRIANGLES,
            bool vertNorms = false, bool faceNorms = false, bool sendCamMats = true);

        // Delete buffers
        void Cleanup();

        // calculate vertex normals
        int calcVertexNormals(GLboolean bFlipNormals = false);

        // calculate the "display" normals
        void calcVertexNormalsForDisplay(GLboolean bFlipNormals = false);

        // calculate texture coordinates
        enum UVType {
            PLANAR_UV = 0,
            CYLINDRICAL_UV,
            SPHERICAL_UV,
            CUBE_MAPPED_UV
        };

        int         calcUVs(Mesh::UVType uvType = Mesh::PLANAR_UV);
        glm::vec2   calcCubeMap(glm::vec3 vEntity);

    private:
        std::vector<glm::vec3> vertexBuffer;
        std::vector<GLuint>    vertexIndices;
        std::vector<glm::vec2> vertexUVs;
        std::vector<glm::vec3> vertexNormals;

        std::vector<glm::vec3> vertexNormalDisplay, faceNormalDisplay;
        std::vector<unsigned> vertNormIndices, faceNormIndices;

        glm::vec3 boundingBox[2];
        GLfloat   normalLength;

        ////           vertex    normal   index
        //VertexBuffer vBuffer, nBuffer, iBuffer;
        VertexArray vArray; // model display
        VertexArray nArray; // normal display
        VertexArray fArray; // face display

        glm::mat4 modelMat;
    };
}

#endif

#include <hyspch.h>

#include <glad/glad.h>
#include "Mesh.h"

#include <gtc/epsilon.hpp>
#include <detail/type_mat4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <gtx/hash.hpp>

#include <tiny_obj_loader.h>

namespace Hayase
{


    /*
  Description: Create a default mesh with no normal length
  and a default model matrix
*/
    Mesh::Mesh()
        : normalLength(0.0f)
        , modelMat(glm::mat4(1.0f))
    {
    }

    Mesh::Mesh(std::string p, std::string m)
        : normalLength(0.0f)
        , modelMat(glm::mat4(1.0f))
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, p.c_str(), m.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        vertexBuffer.clear();
        vertexNormalDisplay.clear();
        vertexIndices.clear();

        std::unordered_map<glm::vec3, uint32_t> uniqueVerts{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex v{};

                if (index.vertex_index >= 0)
                {
                    v.position =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0)
                {
                    v.normal =
                    {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0)
                {
                    v.uv =
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVerts.count(v.position) == 0)
                {
                    uniqueVerts[v.position] = static_cast<uint32_t>(vertexBuffer.size());
                    vertexBuffer.push_back(v.position);
                    vertexNormals.push_back(v.normal);
                    vertexUVs.push_back(v.uv);
                }

                vertexIndices.push_back(uniqueVerts[v.position]);
            }
        }
    }

    /*
      Description: Create a sphere

      rad - The radius of the sphere
      divisions - The number of subdivisions for this sphere (more is higher quality)
    */
    Mesh* Mesh::CreateSphere(float rad, int divisions)
    {
        Mesh* res = new Mesh();

        float x, y, z, xy;
        float nx, ny, nz;
        float length = 1.0f / rad;

        float pi = glm::pi<float>();

        float sectorStep = 2 * pi / divisions;
        float stackStep = pi / divisions;
        float sectorAngle, stackAngle;

        for (unsigned i = 0; i <= divisions; ++i)
        {
            stackAngle = pi / 2 - i * stackStep;
            xy = rad * cosf(stackAngle);
            z = rad * sinf(stackAngle);

            for (int j = 0; j <= divisions; ++j)
            {
                sectorAngle = j * sectorStep;

                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                res->vertexBuffer.push_back(glm::vec3(x, y, z));

                float nx = x * length;
                float ny = y * length;
                float nz = z * length;

                glm::vec3 sphereNorm = glm::vec3(nx, ny, nz);
                res->vertexNormals.push_back(sphereNorm);
            }
        }


        //for (float theta = 0.0f; theta <= 360.0f;)
        //{
        //    float stepTheta = 360.0f / divisions;
        //
        //    for (float phi = 0.0f; phi <= 180.0f;)
        //    {
        //        float stepPhi = 180.0f / divisions;
        //
        //        float x = rad * sinf(theta) * cosf(phi);
        //        float y = rad * cosf(theta) * sinf(phi);
        //        float z = rad * cosf(theta);
        //
        //        res->vertexBuffer.push_back(glm::vec3(x, y, z));
        //
        //        float nx = x * length;
        //        float ny = y * length;
        //        float nz = z * length;
        //
        //        glm::vec3 sphereNorm = glm::vec3(nx, ny, nz);
        //        res->vertexNormals.push_back(sphereNorm);
        //
        //        phi += stepPhi;
        //    }
        //    theta += stepTheta;
        //}

        unsigned k1, k2;
        for (unsigned i = 0; i < divisions; ++i)
        {
            k1 = i * (divisions + 1);
            k2 = k1 + divisions + 1;

            for (unsigned j = 0; j < divisions; ++j, ++k1, ++k2)
            {
                if (i != 0)
                {
                    res->vertexIndices.push_back(k1);
                    res->vertexIndices.push_back(k2);
                    res->vertexIndices.push_back(k1 + 1);
                }

                if (i != (divisions - 1))
                {
                    res->vertexIndices.push_back(k1 + 1);
                    res->vertexIndices.push_back(k2);
                    res->vertexIndices.push_back(k2 + 1);
                }
            }
        }

        return res;
    }

    /*
      Description: Create an orbiting line

      rad - The radius of the line
      divisions - The number of subdivisions for this line (more is higher quality)
    */
    Mesh* Mesh::CreateLine(float rad, int divisions)
    {
        Mesh* res = new Mesh();

        float pi = glm::pi<float>();
        float v = 2 * pi / divisions;

        for (unsigned i = 0; i < divisions; ++i)
        {
            float angle = pi / 2 - i * v;
            float x = rad * sinf(angle);
            float z = rad * cosf(angle);

            // don't care about y - it's constant with the spheres
            res->vertexBuffer.push_back(glm::vec3(x, 0.0f, z));

            res->vertexIndices.push_back(i);
            res->vertexIndices.push_back(i + 1);
        }
        res->vertexIndices.back() = 0;

        return res;
    }

    /*
      Description: Normalize the values of a mesh to be
      between [-1, 1]. This should occur BEFORE the vertex
      buffer is uploaded to the OpenGL pipeline
    */
    void Mesh::ScaleNegOneToOne()
    {
        std::vector<glm::vec3> temp = vertexBuffer;
        vertexBuffer.clear();

        for (glm::vec3 v : temp)
        {
            glm::vec3 res = -1.0f + (v - boundingBox[0]) * 2.0f / (boundingBox[1] - boundingBox[0]);
            vertexBuffer.push_back(res);
        }
    }

    /*
      Description: Initializes a mesh's data by clearing
      all containers and resetting both normal length and model matrix
    */
    void Mesh::initData()
    {
        vertexBuffer.clear();
        vertexIndices.clear();
        vertexUVs.clear();
        vertexNormals.clear();
        vertexNormalDisplay.clear();
        vertNormIndices.clear();
        faceNormIndices.clear();

        boundingBox[0] = glm::vec3(0.0f);
        boundingBox[1] = glm::vec3(0.0f);

        normalLength = 0.00f;

        modelMat = glm::mat4(1.0f);

        return;
    }

    /*
      Description: Generate the buffers of a mesh

      hasNormals - Specify if this mesh contains normals; this will
      generate the normal buffers intended to be displayed
    */
    void Mesh::GenerateBuffers(bool hasNormals)
    {
        const char* idx = "Index";
        const char* vert = "Vertex";
        const char* norm = "Normal";
        const char* uv = "TexCoords";

        vArray.Generate();
        vArray.Bind();

        vArray[idx] = VertexBuffer(GL_ELEMENT_ARRAY_BUFFER);
        vArray[idx].Generate();
        vArray[idx].Bind();
        vArray[idx].SetData<GLuint>(getIndexBufferSize(), getIndexBuffer(), GL_STATIC_DRAW);

        vArray[vert] = VertexBuffer(GL_ARRAY_BUFFER);
        vArray[vert].Generate();
        vArray[vert].Bind();
        vArray[vert].SetData<glm::vec3>(getVertexBufferSize(), getVertexBufferVec3(), GL_STATIC_DRAW);
        vArray[vert].SetAttPointer<GLfloat>(0, 3, GL_FLOAT, 3, 0);

        vArray[vert].Unbind();

        if (hasNormals)
        {
            vArray[norm] = VertexBuffer(GL_ARRAY_BUFFER);
            vArray[norm].Generate();
            vArray[norm].Bind();
            vArray[norm].SetData<glm::vec3>(getVertexNormalCount(), getVertexNormalsVec3(), GL_STATIC_DRAW);
            vArray[norm].SetAttPointer<GLfloat>(1, 3, GL_FLOAT, 3, 0);
        }

        vArray[norm].Unbind();

        vArray[uv] = VertexBuffer(GL_ARRAY_BUFFER);
        vArray[uv].Generate();
        vArray[uv].Bind();
        vArray[uv].SetData<glm::vec2>(getVertexUVCount(), getVertexUVsVec2(), GL_STATIC_DRAW);
        vArray[uv].SetAttPointer<GLfloat>(2, 2, GL_FLOAT, 2, 0);

        vArray[uv].Unbind();

        vArray.Clear();

        if (hasNormals)
        {
            nArray.Generate();
            nArray.Bind();

            nArray[idx] = VertexBuffer(GL_ELEMENT_ARRAY_BUFFER);
            nArray[idx].Generate();
            nArray[idx].Bind();
            nArray[idx].SetData<GLuint>(getDisplayIndexBufferSize(DisplayType::VERTEX),
                getDisplayIndexBuffer(DisplayType::VERTEX), GL_STATIC_DRAW);

            nArray[vert] = VertexBuffer(GL_ARRAY_BUFFER);
            nArray[vert].Generate();
            nArray[vert].Bind();
            nArray[vert].SetData<glm::vec3>(getDisplayCount(DisplayType::VERTEX),
                getDisplayNormalsVec3(DisplayType::VERTEX), GL_STATIC_DRAW);
            nArray[vert].SetAttPointer<GLfloat>(0, 3, GL_FLOAT, 3, 0);

            nArray[vert].Unbind();

            nArray.Clear();
        }

        fArray.Generate();
        fArray.Bind();

        fArray[idx] = VertexBuffer(GL_ELEMENT_ARRAY_BUFFER);
        fArray[idx].Generate();
        fArray[idx].Bind();
        fArray[idx].SetData<GLuint>(getDisplayIndexBufferSize(DisplayType::FACE),
            getDisplayIndexBuffer(DisplayType::FACE), GL_STATIC_DRAW);

        fArray[vert] = VertexBuffer(GL_ARRAY_BUFFER);
        fArray[vert].Generate();
        fArray[vert].Bind();
        fArray[vert].SetData<glm::vec3>(getDisplayCount(DisplayType::FACE),
            getDisplayNormalsVec3(DisplayType::FACE), GL_STATIC_DRAW);
        fArray[vert].SetAttPointer<GLfloat>(0, 3, GL_FLOAT, 3, 0);

        fArray[vert].Unbind();

        nArray.Clear();

        nArray.Clear();

        //iBuffer = VertexBuffer(GL_ELEMENT_ARRAY_BUFFER);
        //iBuffer.Generate();
        //iBuffer.Bind();
        //iBuffer.SetData<GLuint>(getIndexBufferSize(), getIndexBuffer(), GL_STATIC_DRAW);
        //
        //vBuffer = VertexBuffer(GL_ARRAY_BUFFER);
        //vBuffer.Generate();
        //vBuffer.Bind();
        //vBuffer.SetData<glm::vec3>(getVertexBufferSize(), getVertexBufferVec3(), GL_STATIC_DRAW);
        //vBuffer.SetAttPointer<GLfloat>(0, 3, GL_FLOAT, 3, 0);
        //
        //nBuffer = VertexBuffer(GL_ARRAY_BUFFER);
        //nBuffer.Generate();
        //nBuffer.Bind();
        //nBuffer.SetData<glm::vec3>(getVertexNormalCount(), getVertexNormalsVec3(), GL_STATIC_DRAW);
        //nBuffer.SetAttPointer<GLfloat>(1, 3, GL_FLOAT, 3, 0);
        //
        //nBuffer.Unbind();
    }

    /*
      Description: Update the model matrix of a mesh

      angle - The angle of which to rotate the mesh
      scale - The scale of this mesh
      centroid - The cetroid location of the mesh
    */
    void Mesh::Update(float angle, glm::vec3 scale, glm::vec3 centroid, glm::vec3 angleOfRot)
    {
        modelMat = glm::mat4(1.0f);
        
        modelMat = glm::translate(modelMat, centroid)
            * glm::rotate(modelMat, angle, angleOfRot)
            * glm::scale(modelMat, scale);
    }

    /*
      Description: Draw the mesh

      id - The ID of the mesh
      view - The camera's view matrix
      proj - The camera's projection matrix
      type - The specified OpenGL draw type (LINES or TRIANGLES)
      vertNorms - Choose whether to display this mesh's vertex normals (if applicable)
      faceNorms - Choose whether to display this mesh's face normals (if applicable)
    */
    void Mesh::Draw(GLuint id, glm::mat4 view, glm::mat4 proj, std::vector<std::pair<Texture*, std::string>> textures,
        GLenum type, bool vertNorms, bool faceNorms, bool sendCameraMats)
    {
        glUseProgram(id);

        for (int i = 0; i < textures.size(); ++i)
        {
            textures[i].first->Bind(i);
            glUniform1i(glGetUniformLocation(id, textures[i].second.c_str()), i);
        }

        if (sendCameraMats)
        {
            glUniformMatrix4fv(glGetUniformLocation(id, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
            glUniformMatrix4fv(glGetUniformLocation(id, "view"), 1, GL_FALSE, glm::value_ptr(view));
        }

        glUniformMatrix4fv(glGetUniformLocation(id, "model"), 1, GL_FALSE, glm::value_ptr(getModelMat()));

        vArray.Bind();
        vArray.Draw(type, getIndexBufferSize(), GL_UNSIGNED_INT);
        vArray.Clear();

        if (vertNorms)
        {
            nArray.Bind();
            nArray.Draw(GL_LINES, getDisplayIndexBufferSize(DisplayType::VERTEX), GL_UNSIGNED_INT);
            nArray.Clear();
        }

        if (faceNorms)
        {
            fArray.Bind();
            fArray.Draw(GL_LINES, getDisplayIndexBufferSize(DisplayType::FACE), GL_UNSIGNED_INT);
            fArray.Clear();
        }

        glUseProgram(0);
    }

    /*
      Description: Clean up the mesh's buffers
    */
    void Mesh::Cleanup()
    {
        vArray.Cleanup();
        nArray.Cleanup();
        fArray.Cleanup();
        //iBuffer.Cleanup();
        //vBuffer.Cleanup();
        //iBuffer.Cleanup();
    }

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    /////////////////////////////////////////////
    GLfloat* Mesh::getVertexBuffer()
    {
        return reinterpret_cast<GLfloat*>(vertexBuffer.data());
    }

    glm::vec3* Mesh::getVertexBufferVec3()
    {
        return vertexBuffer.data();
    }

    GLfloat* Mesh::getVertexNormals()
    {
        return reinterpret_cast<GLfloat*>(vertexNormals.data());
    }

    glm::vec3* Mesh::getVertexNormalsVec3()
    {
        return vertexNormals.data();
    }

    GLfloat* Mesh::getVertexUVs()
    {
        return reinterpret_cast<GLfloat*>(vertexUVs.data());
    }

    glm::vec2* Mesh::getVertexUVsVec2()
    {
        return vertexUVs.data();
    }

    GLfloat* Mesh::getDisplayNormals(DisplayType t)
    {
        switch (t)
        {
        case DisplayType::VERTEX:
            return reinterpret_cast<GLfloat*>(vertexNormalDisplay.data());
        case DisplayType::FACE:
            return reinterpret_cast<GLfloat*>(faceNormalDisplay.data());
        }

        // ???
        return nullptr;
    }

    glm::vec3* Mesh::getDisplayNormalsVec3(DisplayType t)
    {
        switch (t)
        {
        case DisplayType::VERTEX:
            return vertexNormalDisplay.data();
        case DisplayType::FACE:
            return faceNormalDisplay.data();
        }

        // ???
        return nullptr;
    }

    unsigned int Mesh::getDisplayCount(DisplayType t)
    {
        switch (t)
        {
        case DisplayType::VERTEX:
            return vertexNormalDisplay.size();
        case DisplayType::FACE:
            return faceNormalDisplay.size();
        }

        return 0;
    }

    GLuint* Mesh::getDisplayIndexBuffer(DisplayType t)
    {
        switch (t)
        {
        case DisplayType::VERTEX:
            return vertNormIndices.data();
        case DisplayType::FACE:
            return faceNormIndices.data();
        }

        return nullptr;
    }

    unsigned int Mesh::getDisplayIndexBufferSize(DisplayType t)
    {
        switch (t)
        {
        case DisplayType::VERTEX:
            return vertNormIndices.size();
        case DisplayType::FACE:
            return faceNormIndices.size();
        }

        return 0;
    }



    GLuint* Mesh::getIndexBuffer()
    {
        return vertexIndices.data();
    }

    ////////////////////////////////////
    ////////////////////////////////////
    ////////////////////////////////////
    unsigned int Mesh::getVertexBufferSize()
    {
        return (unsigned int)vertexBuffer.size();
    }

    unsigned int Mesh::getIndexBufferSize()
    {
        return (unsigned int)vertexIndices.size();
    }

    unsigned int Mesh::getTriangleCount()
    {
        return getIndexBufferSize() / 3;
    }

    unsigned int Mesh::getVertexCount()
    {
        return getVertexBufferSize();
    }

    unsigned int Mesh::getVertexNormalCount()
    {
        return vertexNormals.size();
    }

    unsigned int Mesh::getVertexUVCount()
    {
        return vertexUVs.size();
    }

    glm::vec3  Mesh::getModelScale()
    {
        glm::vec3 scale = boundingBox[1] - boundingBox[0];

        if (scale.x == 0.0)
            scale.x = 1.0;

        if (scale.y == 0.0)
            scale.y = 1.0;

        if (scale.z == 0.0)
            scale.z = 1.0;

        return scale;
    }

    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////

    glm::vec3  Mesh::getModelCentroid()
    {
        return glm::vec3(boundingBox[0] + boundingBox[1]) * 0.5f;
    }

    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////

    glm::vec3 Mesh::getCentroidVector(glm::vec3 vVertex)
    {
        return glm::normalize(vVertex - getModelCentroid());
    }

    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////

    struct compareVec
    {
        bool operator() (const glm::vec3& lhs, const glm::vec3& rhs) const
        {
            /*
                NOTE: Doesn't work. Will crash in certain cases.
            */
            //float V = glm::dot( lhs, rhs );
            //bool bRetCode = glm::epsilonNotEqual( V, 1.0f, FLT_EPSILON);
            //
            //return bRetCode;

            // Instead, directly compare the two vectors and 
            // check if certain indices are equal
            glm::highp_bvec3 result = glm::lessThan(lhs, rhs);

            if (glm::epsilonEqual(lhs.x, rhs.x, FLT_EPSILON))
            {
                if (glm::epsilonEqual(lhs.y, rhs.y, FLT_EPSILON))
                {
                    return result.z;
                }
                else
                {
                    return result.y;
                }
            }
            else
            {
                return result.x;
            }
        }
    };

    /*
      Description: Calculate the vertex normals of a mesh

      bFlipNormals - Choose whether to invert the normals' direction
    */
    int Mesh::calcVertexNormals(GLboolean bFlipNormals)
    {
        int rFlag = -1;

        // vertices and indices must be populated
        if (vertexBuffer.empty() || vertexIndices.empty())
        {
            std::cout << "Cannot calculate vertex normals for empty mesh." << std::endl;
            return rFlag;
        }

        // Pre-built vertex normals
    //    if( !vertexNormals.empty() )
    //    {
    //        std::cout << "Vertex normals specified by the file. Skipping this step." << std::endl;
    //        return rFlag;
    //    }

        setNormalLength(0.3f);

        // Initialize vertex normals
        GLuint numVertices = getVertexCount();

        vertexNormals.resize(numVertices, glm::vec3(0.0f));

        vertexNormalDisplay.resize(numVertices * 2, glm::vec3(0.0f));
        faceNormalDisplay.resize(getTriangleCount() * 2, glm::vec3(0.0f));

        std::vector< std::set<glm::vec3, compareVec>>  vNormalSet;
        vNormalSet.resize(numVertices);

        // For every face
        glm::uint index = 0;
        unsigned int j = 0, k = 0;

        for (; index < vertexIndices.size();)
        {
            GLuint a = vertexIndices.at(index++);
            GLuint b = vertexIndices.at(index++);
            GLuint c = vertexIndices.at(index++);

            glm::vec3 vA = vertexBuffer[a];
            glm::vec3 vB = vertexBuffer[b];
            glm::vec3 vC = vertexBuffer[c];

            // Edge vectors
            glm::vec3 E1 = vB - vA;
            glm::vec3 E2 = vC - vA;

            glm::vec3 N = glm::normalize(glm::cross(E1, E2));

            if (bFlipNormals)
                N = N * -1.0f;

            // find the equi-distant point between all 3 points
            // (for face normal)
            glm::vec3 pt = (vA + vB + vC) / 3.0f;

            faceNormalDisplay[2 * j] = pt;
            faceNormalDisplay[(2 * j) + 1] = pt + (normalLength * N);
            ++j;

            faceNormIndices.push_back(k);
            faceNormIndices.push_back(k + 1);

            k += 2;

            // For vertex a
            vNormalSet.at(a).insert(N);
            vNormalSet.at(b).insert(N);
            vNormalSet.at(c).insert(N);
        }

        // Now sum up the values per vertex
        for (int index = 0, j = 0; index < vNormalSet.size(); ++index)
        {
            glm::vec3  vNormal(0.0f);

            //        if( vNormalSet[index].size() <= 0 )
            //            std::cout << "[Normal Set](" << index << ") Size = " << vNormalSet[index].size() << std::endl;

            auto nIt = vNormalSet[index].begin();
            while (nIt != vNormalSet[index].end())
            {
                vNormal += (*nIt);
                ++nIt;
            }

            // save vertex normal
            vertexNormals[index] = glm::normalize(vNormal);

            // save normal to display
            glm::vec3 vA = vertexBuffer[index];

            vertexNormalDisplay[2 * index] = vA;
            vertexNormalDisplay[(2 * index) + 1] = vA + (normalLength * vertexNormals[index]);

            vertNormIndices.push_back(j);
            vertNormIndices.push_back(j + 1);
            j += 2;
        }

        // success
        rFlag = 0;

        return rFlag;
    }

    /*
      Description: Calculate the vertex normals for display

      bFlipNormals - Choose whther to invert the normals' direction
    */
    void Mesh::calcVertexNormalsForDisplay(GLboolean bFlipNormals)
    {
        GLuint numVertices = getVertexCount();
        vertexNormalDisplay.resize(numVertices * 2, glm::vec3(0.0f));

        for (int iNormal = 0; iNormal < vertexNormals.size(); ++iNormal)
        {
            glm::vec3 normal = vertexNormals[iNormal] * normalLength;

            vertexNormalDisplay[2 * iNormal] = vertexBuffer[iNormal];
            vertexNormalDisplay[(2 * iNormal) + 1] = vertexBuffer[iNormal] + normal;
        }
    }
    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////

    GLfloat& Mesh::getNormalLength()
    {
        return normalLength;
    }

    void Mesh::setNormalLength(GLfloat nLength)
    {
        normalLength = nLength;
    }

    /*
      Description: Calculate the UVs of a mesh

      uvType - The type of UV to calculate
    */
    int Mesh::calcUVs(Mesh::UVType uvType)
    {
        int rFlag = -1;

        // clear any existing UV
        vertexUVs.clear();

        glm::vec3 delta = getModelScale();

        for (int nVertex = 0; nVertex < vertexBuffer.size(); ++nVertex)
        {
            glm::vec3 V = vertexBuffer[nVertex];
            glm::vec2 uv(0.0f);

            glm::vec3 normVertex = glm::vec3((V.x - boundingBox[0].x) / delta.x,
                (V.y - boundingBox[0].y) / delta.y,
                (V.z - boundingBox[0].z) / delta.z);

            //        normVertex = (2.0f * normVertex) - glm::vec3(1.0f);

            glm::vec3 centroidVec = getCentroidVector(V);

            float theta(0.0f);
            float z(0.0f);
            float phi(0.0f);

            switch (uvType)
            {
            case Mesh::PLANAR_UV:
                uv.x = (normVertex.x - (-1.0)) / (2.0);
                uv.y = (normVertex.y - (-1.0)) / (2.0);
                break;

            case Mesh::CYLINDRICAL_UV:
                theta = glm::degrees(static_cast<float>(atan2(centroidVec.y, centroidVec.x)));
                theta += 180.0f;

                z = (centroidVec.z + 1.0f) * 0.5f;

                uv.x = theta / 360.0f;
                uv.y = z;
                break;

            case Mesh::SPHERICAL_UV:
                theta = glm::degrees(static_cast<float>(glm::atan(centroidVec.y, centroidVec.x)));
                theta += 180.0f;

                z = centroidVec.z;
                phi = glm::degrees(glm::acos(z / centroidVec.length()));

                uv.x = theta / 360.0f;
                uv.y = 1.0f - (phi / 180.0f);
                break;

            case Mesh::CUBE_MAPPED_UV:
                //uv = calcCubeMap(centroidVec);
                break;
            }

            vertexUVs.push_back(uv);
        }

        return rFlag;
    }

    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////

    glm::vec2 Mesh::calcCubeMap(glm::vec3 vEntity)
    {
        float x = vEntity.x;
        float y = vEntity.y;
        float z = vEntity.z;

        float absX = abs(x);
        float absY = abs(y);
        float absZ = abs(z);

        int isXPositive = x > 0 ? 1 : 0;
        int isYPositive = y > 0 ? 1 : 0;
        int isZPositive = z > 0 ? 1 : 0;

        float maxAxis, uc, vc;
        glm::vec2 uv = glm::vec2(0.0);

        // POSITIVE X
        if (bool(isXPositive) && (absX >= absY) && (absX >= absZ))
        {
            // u (0 to 1) goes from +z to -z
            // v (0 to 1) goes from -y to +y
            maxAxis = absX;
            uc = -z;
            vc = y;
        }

        // NEGATIVE X
        else if (!bool(isXPositive) && absX >= absY && absX >= absZ)
        {
            // u (0 to 1) goes from -z to +z
            // v (0 to 1) goes from -y to +y
            maxAxis = absX;
            uc = z;
            vc = y;
        }

        // POSITIVE Y
        else if (bool(isYPositive) && absY >= absX && absY >= absZ)
        {
            // u (0 to 1) goes from -x to +x
            // v (0 to 1) goes from +z to -z
            maxAxis = absY;
            uc = x;
            vc = -z;
        }

        // NEGATIVE Y
        else if (!bool(isYPositive) && absY >= absX && absY >= absZ)
        {
            // u (0 to 1) goes from -x to +x
            // v (0 to 1) goes from -z to +z
            maxAxis = absY;
            uc = x;
            vc = z;
        }

        // POSITIVE Z
        else if (bool(isZPositive) && absZ >= absX && absZ >= absY)
        {
            // u (0 to 1) goes from -x to +x
            // v (0 to 1) goes from -y to +y
            maxAxis = absZ;
            uc = x;
            vc = y;
        }

        // NEGATIVE Z
        else if (!bool(isZPositive) && absZ >= absX && absZ >= absY)
        {
            // u (0 to 1) goes from +x to -x
            // v (0 to 1) goes from -y to +y
            maxAxis = absZ;
            uc = -x;
            vc = y;
        }

        // Convert range from -1 to 1 to 0 to 1
        uv.s = 0.5f * (uc / maxAxis + 1.0f);
        uv.t = 0.5f * (vc / maxAxis + 1.0f);

        return uv;
    }
}

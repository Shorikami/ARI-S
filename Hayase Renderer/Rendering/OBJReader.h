#ifndef OBJREADER_H
#define OBJREADER_H

#include <string>
#include <fstream>
#include <vector>

// for OpenGL datatypes
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Mesh.h"

namespace Hayase
{
    class OBJReader
    {

    public:
        OBJReader();
        virtual ~OBJReader();

        // initialize the data
        void initData();


        // Read data from a file
        enum ReadMethod { LINE_BY_LINE, BLOCK_IO };
        double ReadOBJFile(std::string filepath,
            Mesh* pMesh, bool scale = true,
            ReadMethod r = ReadMethod::LINE_BY_LINE,
            GLboolean bFlipNormals = false);

    private:

        // Read OBJ file line by line
        int ReadOBJFile_LineByLine(std::string filepath);

        // Read the OBJ file in blocks -- works for files smaller than 1GB
        int ReadOBJFile_BlockIO(std::string filepath);

        // Parse individual OBJ record (one line delimited by '\n')
        void ParseOBJRecord(char* buffer, glm::vec3& min, glm::vec3& max);

        // data members
        Mesh* _currentMesh;
    };
}

#endif
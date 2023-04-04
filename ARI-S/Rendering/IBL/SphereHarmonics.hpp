#ifndef SPHEREHARMONICS_HPP
#define SPHEREHARMONICS_HPP

#include <vector>
#include <glm.hpp>
#include <gtc/matrix_access.hpp>
#include <gtc/type_ptr.hpp>
#include <glad/glad.h>

#include "HDRLoader.hpp"

namespace ARIS
{
    struct SH9
    {
        std::vector<float> result;
    };

    struct SH9Color
    {
        std::vector<glm::vec3> results;
    };

    class SphereHarmonics
    {
    public:

        static float EvaluateBasisFunction(const glm::vec3& N, int idx)
        {
            //switch (idx)
            //{
            //case 0:
            //    return 0.282095f;
            //case 1:
            //    return 0.488603f * N.y;
            //case 2:
            //    return 0.488603f * N.z;
            //case 3:
            //    return 0.488603f * N.x;
            //case 4:
            //    return 1.092548f * N.x * N.y;
            //case 5:
            //    return 1.092548f * N.y * N.z;
            //case 6:
            //    return 0.315392f * (3.0f * N.z * N.z - 1.0f);
            //case 7:
            //    return 1.092548f * N.x * N.z;
            //case 8:
            //    return 0.546274f * (N.x * N.x - N.y * N.y);
            //}

            SH9 res;
            res.result.resize(9);
            
            res.result[0] = 0.5f * sqrtf(1.0f / glm::pi<float>());
            
            float val = 0.5f * sqrtf(3.0f / glm::pi<float>());
            res.result[1] = val * N.y;
            res.result[2] = val * N.z;
            res.result[3] = val * N.x;
            
            res.result[4] = 0.5f * sqrtf(15.0f / glm::pi<float>()) * N.x * N.y;
            res.result[5] = 0.5f * sqrtf(15.0f / glm::pi<float>()) * N.y * N.z;
            res.result[6] = 0.25f * sqrtf(5.0f / glm::pi<float>()) * (3.0f * powf(N.z, 2.0f) - 1.0f);
            res.result[7] = 0.5f * sqrtf(15.0f / glm::pi<float>()) * N.x * N.z;
            res.result[8] = 0.25f * sqrtf(15.0f / glm::pi<float>()) * (powf(N.x, 2.0f) - powf(N.y, 2.0f));
            
            return res.result[idx];
        }

        static SH9Color GenerateLightingCoefficients(Texture hdrMap)
        {
            SH9Color res;
            res.results.clear();
            res.results.resize(9);

            std::vector<float> image;
            int w = 0;
            int h = 0;
            float test = 0.0f;

            float PI = 3.14159265f;

            HDRLoader::ReadHDR(hdrMap.m_Path, image, w, h);

            float thetaStep = PI / static_cast<float>(h);
            float phiStep = 2.0f * PI / static_cast<float>(w);

    #pragma omp parallel for schedule(dynamic, 1) // Magic: Multi-thread y loop
            for (int yy = 0; yy < h; ++yy)
            {
                for (int xx = 0; xx < w; ++xx)
                {
                    float theta = PI * (static_cast<float>(yy) + 0.5f) / static_cast<float>(h);
                    float phi = 2.0f * PI * (static_cast<float>(xx) + 0.5f) / static_cast<float>(w);
                    float sineTerm = sinf(theta);

                    glm::vec3 N = glm::vec3(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));

                    int currOffset = (yy * w + xx) * 3;
                    glm::vec3 px = glm::vec3(image[currOffset + 0], image[currOffset + 1], image[currOffset + 2]);

                    for (unsigned i = 0; i < 9; ++i)
                    {
                        res.results[i] += px * EvaluateBasisFunction(N, i) * sineTerm * thetaStep * phiStep;
                    }

                    test += (sineTerm * thetaStep * phiStep);
                }
            }

            std::vector<float> a;
            a.push_back(PI);

            a.push_back(0.667f * PI);
            a.push_back(0.667f * PI);
            a.push_back(0.667f * PI);

            a.push_back(0.25f * PI);
            a.push_back(0.25f * PI);
            a.push_back(0.25f * PI);
            a.push_back(0.25f * PI);
            a.push_back(0.25f * PI);

            for (unsigned i = 0; i < 9; ++i)
            {
                res.results[i] *= a[i];
            }

            //HDRLoader::WriteHDR("Content/Assets/Textures/HDR/Output.hdr", image, w, h);

            return res;
        }
    };
}

#endif
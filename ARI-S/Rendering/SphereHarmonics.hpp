#ifndef SPHEREHARMONICS_HPP
#define SPHEREHARMONICS_HPP

#include <vector>
#include <glm.hpp>
#include <gtc/matrix_access.hpp>
#include <gtc/type_ptr.hpp>
#include <glad/glad.h>

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
        static glm::vec3 CalculateDirection(int x, int y, int face, int resolution)
        {
            float u = ((float(x) + 0.5f) / float(resolution)) * 2.0f - 1.0f;
            float v = ((float(y) + 0.5f) / float(resolution)) * 2.0f - 1.0f;

            glm::vec3 res = glm::vec3(0.0f);

            switch (face)
            {
            case 0:
                res.x = 1.0f;
                res.y = -v;
                res.z = -u;
                break;
            case 1:
                res.x = -1.0f;
                res.y = -v;
                res.z = u;
                break;
            case 2:
                res.x = u;
                res.y = 1.0f;
                res.z = v;
                break;
            case 3:
                res.x = u;
                res.y = 1.0f;
                res.z = -v;
                break;
            case 4:
                res.x = u;
                res.y = -v;
                res.z = 1.0f;
                break;
            case 5:
                res.x = -u;
                res.y = -v;
                res.z = -1.0f;
                break;
            }

            return glm::normalize(res);
        }

        static SH9 GenerateSHCoefficients(const glm::vec3& N)
        {
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

            return res;
        }

        static SH9Color GenerateLightingCoefficientsNormal(const glm::vec3& N, const glm::vec3& L)
        {
            SH9 coeffs = GenerateSHCoefficients(N);
            SH9Color res;
            res.results.resize(9);

            for (size_t i = 0; i < res.results.size(); ++i)
            {
                res.results[i] = L * coeffs.result[i];
            }

            return res;
        }

        static SH9Color GenerateLightingCoefficients(GLuint tex, int resolution)
        {
            SH9Color res;
            for (size_t i = 0; i < 9; ++i)
            {
                res.results.push_back(glm::vec3(0.0f));
            }

            float weight, weightSum = 0.0f;

            float* img = new float[3 * resolution * resolution];
            glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
            for (int s = 0; s < 6; ++s)
            {
                glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + s, 0, GL_RGB, GL_FLOAT, img);
#pragma omp parallel for schedule(dynamic, 1) // Magic: Multi-thread y loop
                for (int x = 0; x < resolution; ++x)
                {
                    for (int y = 0; y < resolution; ++y)
                    {
                        int r = 3 * x;
                        int c = 3 * y * resolution;
                        glm::vec3 px = glm::vec3(img[r + c], img[r + c + 1], img[r + c + 2]);

                        float u = (x + 0.5f) / resolution;
                        float v = (y + 0.5f) / resolution;
                        u = u * 2.0f - 1.0f;
                        v = v * 2.0f - 1.0f;

                        float temp = 1.0f + pow(u, 2) + pow(v, 2);
                        weight = 4.0f / (sqrt(temp) * temp);

                        glm::vec3 N = CalculateDirection(u, v, s, resolution);

                        for (unsigned i = 0; i < res.results.size(); ++i)
                        {
                            res.results[i] += GenerateLightingCoefficientsNormal(N, px).results[i] * weight;
                        }

                        weightSum += weight;
                    }
                }
            }
            delete[] img;

            for (unsigned i = 0; i < res.results.size(); ++i)
            {
                res.results[i] *= 4.0f * glm::pi<float>() / weightSum;
            }

            return res;
        }
    };
}

#endif
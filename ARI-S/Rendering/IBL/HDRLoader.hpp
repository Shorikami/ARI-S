#ifndef HDRLOADER_HPP
#define HDRLOADER_HPP

#include "math.h"
#include <vector>
#include <iostream>
#include <string.h>
#include <assert.h> 
using namespace std;

#include "rgbe.h"

namespace ARIS
{
    class HDRLoader
    {
    public:
        // Read an HDR image in .hdr (RGBE) format.
        static void ReadHDR(const string inName, std::vector<float>& image,
            int& width, int& height)
        {
            rgbe_header_info info;
            char errbuf[100] = { 0 };

            // Open file and read width and height from the header
            FILE* fp = fopen(inName.c_str(), "rb");
            if (!fp) {
                printf("Can't open file: %s\n", inName.c_str());
                exit(-1);
            }
            int rc = RGBE_ReadHeader(fp, &width, &height, &info, errbuf);
            if (rc != RGBE_RETURN_SUCCESS) {
                printf("RGBE read error: %s\n", errbuf);
                exit(-1);
            }

            // Allocate enough memory
            image.resize(3 * width * height);

            // Read the pixel data and close the file
            rc = RGBE_ReadPixels_RLE(fp, &image[0], width, height, errbuf);
            if (rc != RGBE_RETURN_SUCCESS) {
                printf("RGBE read error: %s\n", errbuf);
                exit(-1);
            }
            fclose(fp);

            printf("Read %s (%dX%d)\n", inName.c_str(), width, height);
        }

        // Write an HDR image in .hdr (RGBE) format.
        static void WriteHDR(const string outName, std::vector<float>& image,
            const int width, const int height)
        {
            rgbe_header_info info;
            char errbuf[100] = { 0 };

            // Open file and write width and height to the header
            FILE* fp = fopen(outName.c_str(), "wb");
            int rc = RGBE_WriteHeader(fp, width, height, NULL, errbuf);
            if (rc != RGBE_RETURN_SUCCESS) {
                printf("RGBE write error: %s\n", errbuf);
                exit(-1);
            }

            // Write the pixel data and close the file
            rc = RGBE_WritePixels_RLE(fp, &image[0], width, height, errbuf);
            if (rc != RGBE_RETURN_SUCCESS) {
                printf("RGBE write error: %s\n", errbuf);
                exit(-1);
            }
            fclose(fp);

            printf("Wrote %s (%dX%d)\n", outName.c_str(), width, height);
        }
    };
}
#endif
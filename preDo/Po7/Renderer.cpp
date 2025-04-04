//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00016;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
//void Renderer::Render(const Scene& scene)
//{
//    std::vector<Vector3f> framebuffer(scene.width * scene.height);
//
//    float scale = tan(deg2rad(scene.fov * 0.5));
//    float imageAspectRatio = scene.width / (float)scene.height;
//    Vector3f eye_pos(278, 273, -800);
//    int m = 0;
//
//    // change the spp value to change sample ammount
//    int spp = 4;
//    std::cout << "SPP: " << spp << "\n";
//    for (uint32_t j = 0; j < scene.height; ++j) {
//        for (uint32_t i = 0; i < scene.width; ++i) {
//            // generate primary ray direction
//            float x = (2 * (i + 0.5) / (float)scene.width - 1) *
//                      imageAspectRatio * scale;
//            float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
//
//            Vector3f dir = normalize(Vector3f(-x, y, 1));
//            for (int k = 0; k < spp; k++){
//                framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0) / spp;  
//            }
//            m++;
//        }
//        UpdateProgress(j / (float)scene.height);
//    }
//    UpdateProgress(1.f);
//
//    // save framebuffer to file
//    FILE* fp;
//    fopen_s(&fp,"binary.ppm", "wb");
//    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
//    for (auto i = 0; i < scene.height * scene.width; ++i) {
//        static unsigned char color[3];
//        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
//        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
//        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
//        fwrite(color, 1, 3, fp);
//    }
//    fclose(fp);   
//}

void RenderTile(const Scene& scene, std::vector<Vector3f>& framebuffer, int start, int end, int spp, std::atomic<int>& progress)
{
    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);

    for (int m = start; m < end; ++m) {
        int i = m % scene.width;
        int j = m / scene.width;

        float x = (2 * (i + 0.5) / (float)scene.width - 1) * imageAspectRatio * scale;
        float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

        Vector3f dir = normalize(Vector3f(-x, y, 1));
        for (int k = 0; k < spp; k++) {
            framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0) / spp;
        }
        progress++;
    }
}

void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    int spp = 16;
    std::cout << "SPP: " << spp << "\n";

    int numThreads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> progress(0);

    int tileSize = (scene.width * scene.height) / numThreads;
    for (int t = 0; t < numThreads; ++t) {
        int start = t * tileSize;
        int end = (t == numThreads - 1) ? (scene.width * scene.height) : (start + tileSize);
        threads.emplace_back(RenderTile, std::ref(scene), std::ref(framebuffer), start, end, spp, std::ref(progress));
    }

    while (progress < scene.width * scene.height) {
        UpdateProgress(progress / (float)(scene.width * scene.height));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    UpdateProgress(1.f);

    FILE* fp;
    fopen_s(&fp, "./Po7/binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}

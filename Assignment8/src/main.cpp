#include "CGL/CGL.h"
#include "CGL/viewer.h"

#include "application.h"
typedef uint32_t gid_t;

#include <iostream>

using namespace std;
using namespace CGL;

void usage(const char *binaryName) {
    printf("Usage: %s [options] <scenefile>\n", binaryName);
    printf("Program Options:\n");
    printf("  -m  <FLOAT>            Mass per node\n");
    printf("  -g  <FLOAT> <FLOAT>    Gravity vector (x, y)\n");
    printf("  -s  <INT>              Number of steps per simulation frame\n");
    printf("\n");
}

int main(int argc, char **argv) {
    AppConfig config;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            config.mass = atof(argv[++i]);
        }
        else if (strcmp(argv[i], "-g") == 0 && i + 2 < argc) {
            config.gravity = Vector2D(atof(argv[++i]), atof(argv[++i]));
        }
        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            config.steps_per_frame = atoi(argv[++i]);
        }
        else {
            usage(argv[0]);
            return 1;
        }
    }
    config.steps_per_frame = 64;

  // create application
    Application *app = new Application(config);

    // create viewer
    Viewer viewer = Viewer();

    // set renderer
    viewer.set_renderer(app);

    // init viewer
    viewer.init();

    // start viewer
    viewer.start();

    return 0;
}

#include "scene_parser.h"
#include "camera.h"
#include "object3d.h"
#include "image.h"
#include "light.h"
#include "raytracer.h"
#define GLUT_DISABLE_ATEXIT_HACK
#include "glCanvas.h"
#include <cstring>
#include <iostream>
#include <fstream>

using namespace std;

// Initialize
// Parameters settings
char* input_file = NULL;
int width = 100;//Ĭ?Ͽ?
int height = 100;//Ĭ?ϸ?
char* output_file = NULL;
float depth_min = 0;
float depth_max = 1;
char* depth_file = NULL;
char* norm_file = NULL;
bool FLAG_SHADE_BACK = false;
bool gui = false;
int theta_steps = 0;
int phi_steps = 0;
bool gouraud = false;

int main(int argc, char** argv) {

	// Argument read-in
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-input")) {
			i++; assert(i < argc);
			input_file = argv[i];
		}
		else if (!strcmp(argv[i], "-size")) {
			i++; assert(i < argc);
			width = atoi(argv[i]);
			i++; assert(i < argc);
			height = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-output")) {
			i++; assert(i < argc);
			output_file = argv[i];
		}
		else if (!strcmp(argv[i], "-depth")) {
			i++; assert(i < argc);
			depth_min = atof(argv[i]);
			i++; assert(i < argc);
			depth_max = atof(argv[i]);
			i++; assert(i < argc);
			depth_file = argv[i];
		}
		else if (!strcmp(argv[i], "-normals")) {
			i++; assert(i < argc);
			norm_file = argv[i];
		}
		else if (!strcmp(argv[i], "-shade_back")) {
			//i++; assert(i < argc);
			FLAG_SHADE_BACK = true;
		}
		else if (!strcmp(argv[i], "-gui")) {
			gui = true;
		}
		else if (!strcmp(argv[i], "-tessellation")) {
			i++;
			assert(i < argc);
			theta_steps = atoi(argv[i]);
			i++;
			assert(i < argc);
			phi_steps = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-gouraud")) {
			gouraud = true;
		}
		else {
			printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
			assert(0);
		}
	}

	//Implement RayTracer
	//rayTracer myraytracer(input_file, output_file, depth_file,norm_file, width, height,depth_min,depth_max, FLAG_SHADE_BACK, gui, theta_steps, phi_steps, gouraud);
	rayTracer myraytracer(input_file, output_file, depth_file, norm_file, width, height, depth_min, depth_max, FLAG_SHADE_BACK, gui);
	myraytracer.render();
	myraytracer.imwrite();

}
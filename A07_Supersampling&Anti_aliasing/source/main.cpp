#include "scene_parser.h"
#include "camera.h"
#include "object3d.h"
#include "image.h"
#include "light.h"
#include "renderer.h"
#define GLUT_DISABLE_ATEXIT_HACK
#include "glCanvas.h"
#include <cstring>
#include <iostream>
#include <fstream>

using namespace std;

// Initialize
// Parameters settings
SceneParser *scene;
char* input_file = NULL;
int width = 100;//д╛хо©М
int height = 100;//д╛хо╦ъ
char* output_file = NULL;
float depth_min = 0;
float depth_max = 1;
char* depth_file = NULL;
char* norm_file = NULL;
bool SHADE_BACK = false;
bool gui = false;
bool shadows = false;
int theta_steps = 0;
int phi_steps = 0;
bool gouraud = false;
int max_bounces = 2;
int cutoff_weight = 0.1;
bool visualize_grid = false;
int nx = 0, ny = 0, nz = 0;
bool stats = false;

int num_samples = 1;
bool random_samples = false;
bool uniform_samples = false;
bool jittered_samples = false;
char* samples_file = NULL;
char* filter_file = NULL;
int zoom_factor = 0;
bool box_filter = false;
bool tent_filter = false;
bool gaussian_filter = false;
float filter_radius = 0;


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
			SHADE_BACK = true;
		}
		else if (!strcmp(argv[i], "-shadows")) {
			shadows = true;
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
		else if (!strcmp(argv[i], "-bounces")) {
            i++;
            assert(i < argc);
            max_bounces = atoi(argv[i]);
        } 
		else if (!strcmp(argv[i], "-weight")) {
            i++;
            assert(i < argc);
            cutoff_weight = atof(argv[i]);
        } 
		else if (!strcmp(argv[i], "-grid")) {
			i++;
			assert(i < argc);
			nx = atoi(argv[i]);
			i++;
			assert(i < argc);
			ny = atoi(argv[i]);
			i++;
			assert(i < argc);
			nz = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-visualize_grid")) {
			visualize_grid = true;
		}
		else if (!strcmp(argv[i], "-stats")) {
			stats = true;
		}
		else if (!strcmp(argv[i], "-random_samples")) {
			random_samples = true;
			i++;
			assert(i < argc);
			num_samples = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-uniform_samples")) {
			uniform_samples = true;
			i++;
			assert(i < argc);
			num_samples = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-jittered_samples")) {
			jittered_samples = true;
			i++;
			assert(i < argc);
			num_samples = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-render_samples")) {
			i++;
			assert(i < argc);
			samples_file = argv[i];
			i++;
			assert(i < argc);
			zoom_factor = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-box_filter")) {
			box_filter = true;
			i++;
			assert(i < argc);
			filter_radius = atof(argv[i]);
		}
		else if (!strcmp(argv[i], "-tent_filter")) {
		tent_filter = true;
		i++;
		assert(i < argc);
		filter_radius = atof(argv[i]);
		}
		else if (!strcmp(argv[i], "-gaussian_filter")) {
		gaussian_filter = true;
		i++;
		assert(i < argc);
		filter_radius = atof(argv[i]);
		}
		else if (!strcmp(argv[i], "-render_filter")) {
		i++;
		assert(i < argc);
		filter_file = argv[i];
		i++;
		assert(i < argc);
		zoom_factor = atoi(argv[i]);
		}
		else {
			printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
			assert(0);
		}
	}
	glutInit(&argc, argv);

	//Implement Renderer
	Renderer myRenderer;
	//myRenderer.render();
	// Render with ray tracing:
	myRenderer.renderRT();
}
#include "scene_parser.h"
#include "camera.h"
#include "object3d.h"
#include "image.h"
#include "light.h"
#include "raytracer.h"
#include <cstring>
#include <iostream>
int main(int argc, char** argv) {

	/*输入-reads the scene*/
	//using the parsing code provided
	char* input_file = NULL;
	int width = 100;//默认宽
	int height = 100;//默认高
	char* output_file = NULL;
	float depth_min = 0;
	float depth_max = 1;
	char* depth_file = NULL;
	char* norm_file = NULL;
	bool FLAG_SHADE_BACK = false;

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
		else {
			printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
			assert(0);
		}
	}

	rayTracer myraytracer(input_file, output_file, depth_file,norm_file, width, height,depth_min,depth_max, FLAG_SHADE_BACK);
	//myraytracer.imwrite();
	myraytracer.render();
	myraytracer.imwrite();

}
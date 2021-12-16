#pragma once
#include "scene_parser.h"
#include "camera.h"
#include "object3d.h"
#include "image.h"
#include "light.h"

class rayTracer {
public:
	int _width;
	int _height;
	float _depth_min;
	float _depth_max;
	bool _SHADE_BACK;
	char *_output_file;
	char *_depth_file;
	char *_norm_file;
	Vec3f COLOR_BLACK;
	Vec3f COLOR_WHITE;
	Vec3f ambientColor;
	SceneParser* scene;
	Camera* camera;
	Group* group;
	Image* image;
	Image* depthImage;
	Image* normalImage;
public:
	rayTracer(char* input_file, char* output_file, char* depth_file, char* norm_file, int width, int height, float depth_min, float depth_max, bool SHADE_BACK);
	~rayTracer();
	
	void render();
	void shader(Image &image, Image &normalImage, Image &depthImage, float i, float j, Hit &hit, Ray &ray);
	bool imwrite();
};



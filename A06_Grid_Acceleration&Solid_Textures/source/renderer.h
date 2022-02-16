#pragma once
#include "scene_parser.h"
#include "glCanvas.h"
#include "camera.h"
#include "object3d.h"
#include "image.h"
#include "light.h"
#include "raytracer.h"

class Renderer {
public:
	Vec3f ambientColor;
	SceneParser* scene;
	Camera* camera;
	Group* group;
	Image* image;
	Image* depthImage;
	Image* normalImage;
	GLCanvas* canvas;
public:
	Renderer();
	~Renderer();
	
	void render();
	void renderRT();
	void shader(Image &image, Image &normalImage, Image &depthImage, float i, float j, Hit &hit, Ray &ray);
};



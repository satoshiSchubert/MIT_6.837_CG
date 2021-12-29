#include "Renderer.h"
#include <iostream>
#include <algorithm>
#include <vector>
extern SceneParser *scene;
extern int max_bounces;
extern int cutoff_weight;

void renderFunction() {}

void traceRayFunction(float x, float y) {
	Ray ray = scene->getCamera()->generateRay(Vec2f(x, y));
	RayTracer rayTracer(scene, max_bounces, cutoff_weight);
	float tmin = 0.001f;
	Hit hit(INFINITY);
	Vec3f color = rayTracer.traceRay(ray, tmin, 0, 1.0, hit);
}
// Constructor.
//Renderer::Renderer(char* input_file, char* output_file, char* depth_file, char* norm_file, int width, int height, float depth_min, float depth_max, bool SHADE_BACK = false, bool gui = false, int theta_steps=5, int phi_steps=5, bool gouraud=false){
Renderer::Renderer(char* input_file, char* output_file, char* depth_file, char* norm_file, int width, int height, float depth_min, float depth_max, bool SHADE_BACK = false, bool gui = false) {
	_width = width;
	_height = height;
	_depth_min = depth_min;
	_depth_max = depth_max;
	_output_file = output_file;
	_depth_file = depth_file;
	_norm_file = norm_file;
	if (SHADE_BACK) _SHADE_BACK = true;
	COLOR_BLACK = Vec3f(0.f, 0.f, 0.f);
	COLOR_WHITE = Vec3f(1.f, 1.f, 1.f);
	scene = new SceneParser(input_file);
	camera = scene->getCamera();
	group = scene->getGroup();
	ambientColor = scene->getAmbientLight();
	image = new Image(width, height);//设置图像大小
	image->SetAllPixels(scene->getBackgroundColor());//图像背景颜色设置为场景背景颜色
	depthImage = new Image(width, height);//设置深度图像
	depthImage->SetAllPixels(COLOR_BLACK);//设置成黑色
	normalImage = new Image(width, height);
	normalImage->SetAllPixels(COLOR_BLACK);
	
	// For OpenGL
	if (gui) {
		GLCanvas canvas;
		canvas.initialize(scene, renderFunction, traceRayFunction);
		return;
	}	
}

void Renderer::renderRT() {
	RayTracer rayTracer(scene, max_bounces, cutoff_weight);
	for (int i = 0; i < _width; i++) {
		for (int j = 0; j < _height; j++) {
			// 0.0 ~ 1.0, 相对坐标.
			float x = float(i) / float(_width);
			float y = float(j) / float(_height);
			Ray ray = camera->generateRay(Vec2f(x, y));
			float tmin = 0.001f;
			Hit hit(INFINITY);
						
			Vec3f color = rayTracer.traceRay(ray, tmin, 0, 1.0, hit);
			image->SetPixel(i, j, color);
		}
	}
}

void Renderer::render() {
	for (int i = 0; i < _width; i++) {
		for (int j = 0; j < _height; j++) {
			// 0.0 ~ 1.0, 相对坐标.
			float x = float(i) / float(_width);
			float y = float(j) / float(_height);
			Ray ray = camera->generateRay(Vec2f(x, y));
			Hit hit(FLT_MAX, nullptr, COLOR_BLACK);
			if (group->intersect(ray, hit, camera->getTMin())) {
				// Do rendering!
				shader(*image, *normalImage, *depthImage, i, j, hit, ray);
			}
		}
	}
}

void Renderer::shader(Image &image, Image &normalImage,Image &depthImage, float i, float j, Hit &hit, Ray &ray) {
	Vec3f v_norm = hit.getNormal();
	// Check if it's inside the surface.
	if (ray.getDirection().Dot3(v_norm)>0) {
		//cout << "INSIDE!" << endl;
		if (_SHADE_BACK)
			v_norm = -1 * v_norm;// negate the norm.
		else {
			image.SetPixel(i, j, COLOR_BLACK);
			//cout << "ERRORSIDE" << endl;
			return; //error side without further operation.
		}
	}
	Material *material = hit.getMaterial();
	Vec3f c_obj = material->getDiffuseColor();
	Vec3f c_ambient = scene->getAmbientLight();
	vector<Light> arrayLight;
	// Calc diffuse shading
	Vec3f c_pixel = c_ambient * c_obj;
	for (int count = 0; count < scene->getNumLights(); count++) {
		Light *light = scene->getLight(count);
		Vec3f p_hit = hit.getIntersectionPoint();
		Vec3f d_light, c_light;
		float distance; // not use
		light->getIllumination(p_hit, d_light, c_light, distance); //HACK:todo!
		// Assignment2中的 漫反射(diffuse)+环境光(ambient)
		//c_pixel += max(v_norm.Dot3(d_light), 0.f)*c_light*c_obj;
		// Assignment3中的 Phong模型：漫反射(diffuse)+环境光(ambient)+镜面反射(specular)
		c_pixel += material->Shade(ray, hit, d_light, c_light);
	}
	image.SetPixel(i, j, c_pixel);

	// get normal image pixel.
	normalImage.SetPixel(i, j, Vec3f(fabs(v_norm.x()), fabs(v_norm.y()), fabs(v_norm.z())));

	// get depth image pixel.
	assert(_depth_max >= _depth_min);
	float t = hit.getT();
	if (t > _depth_max)	t = _depth_max;
	if (t < _depth_min) t = _depth_min;
	t = (_depth_max -t) / (_depth_max - _depth_min);
	depthImage.SetPixel(i, j, Vec3f(t, t, t));
}

bool Renderer::imwrite() {
	// validate the path
	if (_output_file != nullptr) {
		image->SaveTGA(_output_file);
	}
	if (_depth_file != nullptr) {
		depthImage->SaveTGA(_depth_file);
	}
	if (_norm_file != nullptr) {
		normalImage->SaveTGA(_norm_file);
	}
	return 1;
}

// Destructor.
Renderer::~Renderer() { 
	if (scene != NULL)
		delete scene;
	if (camera != NULL)
		delete camera;
	if (group != NULL)
		delete group;
	if (image != NULL)
		delete image;
	if (depthImage != NULL)
		delete depthImage;
	if (normalImage != NULL)
		delete normalImage;
}












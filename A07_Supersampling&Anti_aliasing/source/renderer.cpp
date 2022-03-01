#include "Renderer.h"
#include "raytracing_stats.h"
#include <iostream>
#include <algorithm>
#include <vector>

Vec3f COLOR_BLACK = Vec3f(0.f, 0.f, 0.f);
Vec3f COLOR_WHITE = Vec3f(1.f, 1.f, 1.f);

//参数传递，这里不想继续改Renderer的构造函数了。。。
extern int max_bounces;
extern int cutoff_weight;
extern int nx, ny, nz;
extern float depth_min;
extern float depth_max;
extern bool visualize_grid;
extern bool stats;
extern bool SHADE_BACK;
extern bool gui;
extern char* input_file;
extern char* output_file;
extern char* depth_file;
extern char* norm_file;
extern char* samples_file;
extern int width;
extern int height;
extern bool random_samples;
extern bool uniform_samples;
extern bool jittered_samples;
extern int zoom_factor;
extern int num_samples;
extern bool box_filter;
extern bool tent_filter;
extern bool gaussian_filter;
extern float filter_radius;
extern char* filter_file;

//For Sampling Edition，使能采样
void renderFunction_with_sample() {
	SceneParser* scene = new SceneParser(input_file);
	Camera *camera = scene->getCamera();
	Image image(width, height);
	image.SetAllPixels(scene->getBackgroundColor());
	RayTracer rayTracer(scene, max_bounces, cutoff_weight);
	if (nx != 0) // 使用网格加速
		RayTracingStats::Initialize(width, height, rayTracer.grid->getBoundingBox(), nx, ny, nz);
	else
		RayTracingStats::Initialize(width, height, nullptr, 0, 0, 0);

	//采样
	Film *film = new Film(width, height, num_samples);
	Sampler *sampler = new Sampler(num_samples);
	if (random_samples)
		sampler = new RandomSampler(num_samples);
	if (uniform_samples)
		sampler = new UniformSampler(num_samples);
	if (jittered_samples)
		sampler = new JitteredSampler(num_samples);

	//滤波
	Filter *filter = nullptr;
	if (box_filter) {
		filter = new BoxFilter(filter_radius);
	}
	if (tent_filter)
		filter = new TentFilter(filter_radius);
	if (gaussian_filter)
		filter = new GaussianFilter(filter_radius);

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			RayTracingStats::IncrementNumNonShadowRays();
			//////cout << "checkpoint15" << endl;
					
			//对于每一像素格
			//这里再叠加上使用randomsample的位置
			for (int index = 0; index < num_samples; index++) {
				//////cout << "checkpoint16" << endl;
				Vec2f offset = sampler->getSamplePosition(index);
				// 0.0 ~ 1.0, 相对坐标.
				float x = float(i+ offset.x()) / float(width); 
				float y = float(j+ offset.y()) / float(height);
				//////cout << "checkpoint17" << endl;
				Ray ray = camera->generateRay(Vec2f(x, y));
				float tmin = 0.001f;
				//////cout << "checkpoint18" << endl;
				Hit hit(INFINITY);
				Vec3f color = rayTracer.traceRay(ray, tmin, 0, 1.0, hit);
				//image.SetPixel(i, j, color);
				//////cout << "checkpoint19" << endl;
				//make Film
				film->setSample(i, j, index, offset, color);
				//////cout << "checkpoint20" << endl;
			}
		}
	}

	if (output_file != nullptr) {
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				
				if (filter) {
					//filter->getColor(i, j, film).show();
					image.SetPixel(i, j, filter->getColor(i, j, film));
				}
				else {
					//////cout << "checkpoint32" << endl;
					image.SetPixel(i, j, film->getSample(i, j, 0).getColor());
				}
			}
		}
		image.SaveTGA(output_file);
	}

	if (samples_file!=nullptr)
		film->renderSamples(samples_file, zoom_factor);

	if (filter_file != nullptr)
		film->renderFilter(filter_file, zoom_factor,filter);

	if (stats)
		RayTracingStats::PrintStatistics();
}



// OpenGL GUI界面下，键盘按'r'或'R'会运行renderFunction()
// 这样就不用为了渲染图片而切换到非GUI模式
void renderFunction() {
	SceneParser* scene = new SceneParser(input_file);
	Camera *camera = scene->getCamera();
	Image image(width, height);
	image.SetAllPixels(scene->getBackgroundColor());
	
	RayTracer rayTracer(scene, max_bounces, cutoff_weight);
	if (nx != 0) // 使用网格加速
		RayTracingStats::Initialize(width, height, rayTracer.grid->getBoundingBox(), nx, ny, nz);
	else
		RayTracingStats::Initialize(width, height, nullptr, 0, 0, 0);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			RayTracingStats::IncrementNumNonShadowRays();
			// 0.0 ~ 1.0, 相对坐标.
			float x = float(i) / float(width);
			float y = float(j) / float(height);
			Ray ray = camera->generateRay(Vec2f(x, y));
			float tmin = 0.001f;
			Hit hit(INFINITY);
			Vec3f color = rayTracer.traceRay(ray, tmin, 0, 1.0, hit);
			image.SetPixel(i, j, color);
		}
	}
	if (output_file != nullptr) {
		image.SaveTGA(output_file);
	}
	if(stats)
		RayTracingStats::PrintStatistics();
}

// OpenGL GUI界面下，键盘按't'或'T'会运行traceRayFunction()
void traceRayFunction(float x, float y) {
	SceneParser* scene = new SceneParser(input_file);
	Ray ray = scene->getCamera()->generateRay(Vec2f(x, y));
	RayTracer rayTracer(scene, max_bounces, cutoff_weight);
	float tmin = 0.001f;
	Hit hit(INFINITY);
	Vec3f color = rayTracer.traceRay(ray, tmin, 0, 1.0, hit);
	Hit hit2(INFINITY);
	rayTracer.grid->intersect(ray, hit2, tmin);
}

// Constructor.
Renderer::Renderer() {
	scene = new SceneParser(input_file);
	Grid *grid = nullptr;
	if (nx != 0) {
		grid = new Grid(scene->getGroup()->getBoundingBox(), nx, ny, nz);
		scene->getGroup()->insertIntoGrid(grid, nullptr);
	}
	
	// For OpenGL
	if (gui) {
		GLCanvas canvas;
		canvas.initialize(scene, renderFunction, traceRayFunction, grid, visualize_grid);
		return;
	}	
}

void Renderer::renderRT() {
	//renderFunction();
	renderFunction_with_sample();
}

// DEPRECATED
//void Renderer::render() {
//	for (int i = 0; i < width; i++) {
//		for (int j = 0; j < height; j++) {
//			// 0.0 ~ 1.0, 相对坐标.
//			float x = float(i) / float(width);
//			float y = float(j) / float(height);
//			Ray ray = camera->generateRay(Vec2f(x, y));
//			Hit hit(FLT_MAX, nullptr, COLOR_BLACK);
//			if (group->intersect(ray, hit, camera->getTMin())) {
//				// Do rendering!
//				shader(*image, *normalImage, *depthImage, i, j, hit, ray);
//			}
//		}
//	}
//	if (output_file != nullptr) {
//		image->SaveTGA(output_file);
//	}
//	if (depth_file != nullptr) {
//		depthImage->SaveTGA(depth_file);
//	}
//	if (norm_file != nullptr) {
//		normalImage->SaveTGA(norm_file);
//	}
//}

// DEPRECATED
//void Renderer::shader(Image &image, Image &normalImage,Image &depthImage, float i, float j, Hit &hit, Ray &ray) {
//	Vec3f v_norm = hit.getNormal();
//	// Check if it's inside the surface.
//	if (ray.getDirection().Dot3(v_norm)>0) {
//		//cout << "INSIDE!" << endl;
//		if (SHADE_BACK)
//			v_norm = -1 * v_norm;// negate the norm.
//		else {
//			image.SetPixel(i, j, COLOR_BLACK);
//			//cout << "ERRORSIDE" << endl;
//			return; //error side without further operation.
//		}
//	}
//	Material *material = hit.getMaterial();
//	Vec3f c_obj = material->getDiffuseColor();
//	Vec3f c_ambient = scene->getAmbientLight();
//	vector<Light> arrayLight;
//	// Calc diffuse shading
//	Vec3f c_pixel = c_ambient * c_obj;
//	for (int count = 0; count < scene->getNumLights(); count++) {
//		Light *light = scene->getLight(count);
//		Vec3f p_hit = hit.getIntersectionPoint();
//		Vec3f d_light, c_light;
//		float distance; // not use
//		light->getIllumination(p_hit, d_light, c_light, distance); //HACK:todo!
//		// Assignment2中的 漫反射(diffuse)+环境光(ambient)
//		//c_pixel += max(v_norm.Dot3(d_light), 0.f)*c_light*c_obj;
//		// Assignment3中的 Phong模型：漫反射(diffuse)+环境光(ambient)+镜面反射(specular)
//		c_pixel += material->Shade(ray, hit, d_light, c_light);
//	}
//	image.SetPixel(i, j, c_pixel);
//
//	// get normal image pixel.
//	normalImage.SetPixel(i, j, Vec3f(fabs(v_norm.x()), fabs(v_norm.y()), fabs(v_norm.z())));
//
//	// get depth image pixel.
//	assert(depth_max >= depth_min);
//	float t = hit.getT();
//	if (t > depth_max)	t = depth_max;
//	if (t < depth_min) t = depth_min;
//	t = (depth_max -t) / (depth_max - depth_min);
//	depthImage.SetPixel(i, j, Vec3f(t, t, t));
//}

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












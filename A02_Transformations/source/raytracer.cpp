#include "raytracer.h"
#include <iostream>
#include <algorithm>

// Constructor.
rayTracer::rayTracer(char* input_file, char* output_file, char* depth_file, char* norm_file, int width, int height, float depth_min, float depth_max, bool SHADE_BACK = false){
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

	cout << "Image size: " << _width << " * " << _height << endl;
}

void rayTracer::render() {
	for (int i = 0; i < _width; i++) {
		for (int j = 0; j < _height; j++) {
			
			// 0.0 ~ 1.0, 相对坐标.
			float x = float(i) / float(_width);
			float y = float(j) / float(_height);
			Ray ray = camera->generateRay(Vec2f(x, y));
			Hit hit(FLT_MAX, nullptr, COLOR_BLACK);
			if (group->intersect(ray, hit, camera->getTMin())) {
				
				////DEBUG
				//if (x == 0.5&&y == 0.05) {
				//	cout << "t: " << hit.getT() << endl;
				//	auto temp = ray.getDirection();
				//	auto v_norm = hit.getNormal();
				//	cout << "v_norm z: " << v_norm[2] << "  Rd z: " << temp[2] << endl;
				//	//shader(*image, *normalImage, *depthImage, i, j, hit, ray);

				//}

				// Do rendering!
				shader(*image, *normalImage, *depthImage, i, j, hit, ray);
				
			}
			
		}
	}
}

void rayTracer::shader(Image &image, Image &normalImage,Image &depthImage, float i, float j, Hit &hit, Ray &ray) {
	Vec3f v_norm = hit.getNormal();
	// Check if it's inside the surface.
	if (ray.getDirection().Dot3(v_norm)>0) { //TODO:乘积小于0：outside
		//cout << "INSIDE!" << endl;
		if (_SHADE_BACK)
			v_norm = -1 * v_norm;// negate the norm.
		else {
			image.SetPixel(i, j, COLOR_BLACK);
			//cout << "ERRORSIDE" << endl;
			return; //error side without further operation.
		}
	}
	Material *m = hit.getMaterial();
	Vec3f c_obj = m->getDiffuseColor();
	Vec3f c_ambient = scene->getAmbientLight();
	vector<Light> arrayLight;
	// Calc diffuse shading
	Vec3f c_pixel = c_ambient * c_obj;
	for (int count = 0; count < scene->getNumLights(); count++) {
		Light *light = scene->getLight(count);
		Vec3f p_hit = hit.getIntersectionPoint();
		Vec3f d_light, c_light;
		light->getIllumination(p_hit, d_light, c_light);
		c_pixel += max(v_norm.Dot3(d_light), 0.f)*c_light*c_obj;
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

bool rayTracer::imwrite() {
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
rayTracer::~rayTracer() { 
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












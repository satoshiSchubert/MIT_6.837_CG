#include "scene_parser.h"
#include "camera.h"
#include "object3d.h"
#include "image.h"
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
		else {
			printf("whoops error with command line argument %d: '%s'\n", i, argv[i]);
			assert(0);
		}
	}
	/*初始化场景、相机、对象组及两幅图像*/
	SceneParser scene(input_file);
	Camera* camera = scene.getCamera();
	Group* group = scene.getGroup();
	Image image(width, height);//设置图像大小
	image.SetAllPixels(scene.getBackgroundColor());//图像背景颜色设置为场景背景颜色
	Image depthImage(width, height);//设置深度图像
	depthImage.SetAllPixels(Vec3f(0.0, 0.0, 0.0));//设置成黑色

	/*循环遍历图像平面中的像素，使用OrthographicCamera类生成射线，将其与Group中的各个对象求交点，并将最近点保存到hit中*/
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			float x = float(i) / float(width);
			float y = float(j) / float_t(height);
			Ray ray = camera->generateRay(Vec2f(x, y));//产生射线
			Hit hit(INFINITY, nullptr);//hit存储最近的交点，此处初始t设置为了无穷大
			bool flag = group->intersect(ray, hit, camera->getTMin());//使该条射线和每一个基本图元求交点
			if (flag) {
				image.SetPixel(i, j, hit.getMaterial()->getDiffuseColor());//设置该点像素的颜色
				float t = hit.getT();
				//可视化深度t
				//超过则设置为边界值
				if (t > depth_max) t = depth_max;//此处max=1、min=0
				if (t < depth_min) t = depth_min;
				t = (depth_max - t) / (depth_max - depth_min);//t越小越近，越近的颜色越深
				depthImage.SetPixel(i, j, Vec3f(t, t, t));//设置颜色值
			}
		}
	}
	//异常判断
	if (output_file != nullptr) image.SaveTGA(output_file);
	if (depth_file != nullptr) depthImage.SaveTGA(depth_file);
	return 0;
}
#include "scene_parser.h"
#include "camera.h"
#include "object3d.h"
#include "image.h"
#include <cstring>
#include <iostream>
int main(int argc, char** argv) {

	/*����-reads the scene*/
	//using the parsing code provided
	char* input_file = NULL;
	int width = 100;//Ĭ�Ͽ�
	int height = 100;//Ĭ�ϸ�
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
	/*��ʼ������������������鼰����ͼ��*/
	SceneParser scene(input_file);
	Camera* camera = scene.getCamera();
	Group* group = scene.getGroup();
	Image image(width, height);//����ͼ���С
	image.SetAllPixels(scene.getBackgroundColor());//ͼ�񱳾���ɫ����Ϊ����������ɫ
	Image depthImage(width, height);//�������ͼ��
	depthImage.SetAllPixels(Vec3f(0.0, 0.0, 0.0));//���óɺ�ɫ

	/*ѭ������ͼ��ƽ���е����أ�ʹ��OrthographicCamera���������ߣ�������Group�еĸ��������󽻵㣬��������㱣�浽hit��*/
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			float x = float(i) / float(width);
			float y = float(j) / float_t(height);
			Ray ray = camera->generateRay(Vec2f(x, y));//��������
			Hit hit(INFINITY, nullptr);//hit�洢����Ľ��㣬�˴���ʼt����Ϊ�������
			bool flag = group->intersect(ray, hit, camera->getTMin());//ʹ�������ߺ�ÿһ������ͼԪ�󽻵�
			if (flag) {
				image.SetPixel(i, j, hit.getMaterial()->getDiffuseColor());//���øõ����ص���ɫ
				float t = hit.getT();
				//���ӻ����t
				//����������Ϊ�߽�ֵ
				if (t > depth_max) t = depth_max;//�˴�max=1��min=0
				if (t < depth_min) t = depth_min;
				t = (depth_max - t) / (depth_max - depth_min);//tԽСԽ����Խ������ɫԽ��
				depthImage.SetPixel(i, j, Vec3f(t, t, t));//������ɫֵ
			}
		}
	}
	//�쳣�ж�
	if (output_file != nullptr) image.SaveTGA(output_file);
	if (depth_file != nullptr) depthImage.SaveTGA(depth_file);
	return 0;
}
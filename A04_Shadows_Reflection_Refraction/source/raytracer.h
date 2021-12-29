#include "scene_parser.h"
#include "object3d.h"
#include "light.h"
#include "rayTree.h"
extern bool shadows;
extern bool shade_back;

class RayTracer {
public:
	SceneParser *scene;
	int max_bounces; // ���ߵ���󷴵�����
	float cutoff_weight; // weight:��ǰ���߶����ĳ���ص���ɫ���׵�Ȩ�أ�cutoff_weightӦ�������Ȩ�ص����ֵ

public:
	RayTracer(SceneParser *s, int max_bounces, float cutoff_weight) :
		scene(s),
		max_bounces(max_bounces),
		cutoff_weight(cutoff_weight) {}

	~RayTracer() {}

	Vec3f traceRay(Ray &ray, float tmin, int bounces, float weight, Hit &hit);
};
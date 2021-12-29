#include "scene_parser.h"
#include "object3d.h"
#include "light.h"
#include "rayTree.h"
extern bool shadows;
extern bool shade_back;

class RayTracer {
public:
	SceneParser *scene;
	int max_bounces; // 光线的最大反弹次数
	float cutoff_weight; // weight:当前光线对最后某像素的颜色贡献的权重；cutoff_weight应该是这个权重的最低值

public:
	RayTracer(SceneParser *s, int max_bounces, float cutoff_weight) :
		scene(s),
		max_bounces(max_bounces),
		cutoff_weight(cutoff_weight) {}

	~RayTracer() {}

	Vec3f traceRay(Ray &ray, float tmin, int bounces, float weight, Hit &hit);
};
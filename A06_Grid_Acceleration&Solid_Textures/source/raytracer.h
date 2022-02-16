#include "scene_parser.h"
#include "object3d.h"
#include "light.h"
#include "rayTree.h"
extern bool shadows;
extern bool shade_back;
extern bool visualize_grid;
extern int nx, ny, nz;

class RayTracer {
public:
	SceneParser *scene;
	int max_bounces; // ���ߵ���󷴵�����
	float cutoff_weight; // weight:��ǰ���߶����ĳ���ص���ɫ���׵�Ȩ�أ�cutoff_weightӦ�������Ȩ�ص����ֵ
	Grid *grid;

public:
	RayTracer(SceneParser *s, int max_bounces, float cutoff_weight) :
		scene(s),
		max_bounces(max_bounces),
		cutoff_weight(cutoff_weight) {

		if (nx != 0) {
			//��һ��ֻ�ǳ�ʼ����bbox��û�в���grid
			grid = new Grid(s->getGroup()->getBoundingBox(), nx, ny, nz);
			//��һ���Ų���
			s->getGroup()->insertIntoGrid(grid, nullptr);
		}
		else {
			grid = nullptr;
		}
	}

	~RayTracer() {}

	Vec3f traceRay(Ray &ray, float tmin, int bounces, float weight, Hit &hit);
	Vec3f RayCast(Ray &ray, float tmin, int bounces, float weight, Hit &hit);
	Vec3f RayCastFast(Ray &ray, float tmin, int bounces, float weight, Hit &hit);
};
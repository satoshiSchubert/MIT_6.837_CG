#include "object3d.h"
#include <math.h>
#include <algorithm>

using namespace std;

bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
	Vec3f Ro = r.getOrigin() - _center;
	Vec3f Rd = r.getDirection();
	float b = 2 * Ro.Dot3(Rd);
	float a = Rd.Dot3(Rd);
	float c = Ro.Dot3(Ro) - _radius * _radius;
	float delta = b * b - 4 * a * c;
	if (delta < 0)
		// No hit.
		return false;
	float t;
	float t1 = (-b + sqrt(delta)) / (2 * a);
	float t2 = (-b - sqrt(delta)) / (2 * a);
	float tMin = min(t1, t2);
	float tMax = max(t1, t2);
	
	//if (tMin >= tmin)
	//	t = tMin;
	//else if (tMax >= tmin)
	//	t = tMax; // View Point inside the sphere.
	//�Ƿ���Ϊ��tΪ��ֵʱ������tmin��ȡ�����������߼�����
	if (tMin < h.getT()) {//closer��ǰ���㣬����
		h = Hit(tMin, _material);
	}
	return true;
}

Sphere::Sphere(Vec3f &center, float &radius, Material* material) :_center(center), _radius(radius) {
	// ����ĳ�ʼ���б��ܳ�ʼ������ĳ�Ա
	//https://blog.csdn.net/yanlei208/article/details/101089966
	_material = material; //�������Ǹ�ֵ��ʼ��
}


Group::Group(const int num) :_n(num) {};

void Group::addObject(int index, Object3D *obj) {
	assert(index < _n);
	objArray.push_back(obj);
}

bool Group::intersect(const Ray &r, Hit &h, float tmin) {
	bool success = false;
	for (auto obj : objArray) {
		if (obj->intersect(r, h, tmin)) {
			success = true;
		}
	}
	return success;
}

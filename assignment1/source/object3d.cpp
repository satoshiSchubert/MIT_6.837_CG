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
	//是否因为当t为负值时，大于tmin（取补），导致逻辑出错
	if (tMin < h.getT()) {//closer当前交点，更新
		h = Hit(tMin, _material);
	}
	return true;
}

Sphere::Sphere(Vec3f &center, float &radius, Material* material) :_center(center), _radius(radius) {
	// 子类的初始化列表不能初始化父类的成员
	//https://blog.csdn.net/yanlei208/article/details/101089966
	_material = material; //这里则是赋值初始化
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

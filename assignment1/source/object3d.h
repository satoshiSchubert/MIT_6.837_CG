#pragma once

#include "ray.h"
#include "hit.h"
#include "material.h"
#include <vector>

// Object3D
class Object3D {

public:
	Material* _material = nullptr;
	virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;
};

// Sphere
class Sphere : public Object3D {
private:
	Vec3f _center;
	float _radius;
public:
	Sphere(Vec3f &center, float &radius, Material* material);
	bool intersect(const Ray &r, Hit &h, float tmin);
};


// Group
class Group :public Object3D {
public:
	int _n; // num of object3d
	vector<Object3D*> objArray;
	Group(const int num);
	void addObject(int index, Object3D *obj);
	bool intersect(const Ray &r, Hit &h, float tmin);
};




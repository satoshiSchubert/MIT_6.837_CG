#pragma once

#include "ray.h"
#include "hit.h"
#include "material.h"
#include "matrix.h"
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

// Plane
class Plane :public Object3D {
public:
	Vec3f _v_norm;
	float _d;
public:
	Plane(Vec3f &normal, float d, Material *m);
	Plane(Vec3f &p0, Vec3f &p1, Vec3f &p2, Material *m);
	bool intersect(const Ray &r, Hit &h, float tmin);
};

//Triangle
class Triangle :public Object3D {
private:
	Vec3f _pa;
	Vec3f _pb;
	Vec3f _pc;
	Vec3f _v_norm;
public:
	Triangle(Vec3f &a, Vec3f &b, Vec3f &c, Material *m);
	bool intersect(const Ray &r, Hit &h, float tmin);
};

//Transform
class Transform :public Object3D {
private:
	Object3D *_obj;
	Matrix _mat;
public:
	Transform(Matrix &mat, Object3D *o);
	bool intersect(const Ray &r, Hit &h, float tmin);
};













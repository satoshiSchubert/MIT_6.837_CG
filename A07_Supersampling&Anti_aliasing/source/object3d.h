#pragma once

#include "ray.h"
#include "hit.h"
#include "material.h"
#include "matrix.h"
#include "boundingbox.h"
#include "marchinginfo.h"
#include <GL/freeglut.h>
#include <vector>
#include <set>

//这里需要提前声明一下Grid
class Grid;
// Object3D
class Object3D {

public:
	Material* _material = nullptr;
	BoundingBox* _bbox = nullptr;
	virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;
	virtual bool is_triangle() { return false; }
	virtual BoundingBox *getBoundingBox() { return _bbox; }
	virtual BoundingBox *getTriangleBoundingBox(Matrix *m){ return _bbox; } //use a special case for transformed triangles
	virtual void insertIntoGrid(Grid *g, Matrix *m);
	virtual void paint(void) = 0;
};

// Grid
class Grid :public Object3D {
public:
	int _nx, _ny, _nz;
	//vector<bool> opaque;
	vector<vector<Object3D*>> opaque;//维护一个vector，这个vector中的某一格可能属于多个primitives，所以还要套一层vector
	set<Object3D*> is_intersected;
	vector<Object3D*> infinitePrimitives;
	//std::set作为标准库的一个关联容器，实现内部元素进行了排序，使用这特性可以对一组元素进行插入排序。
	//std::set最初的设计是完成数学中“集合”的概念，它提供的接口也是如此。

	Grid(BoundingBox *bb, int nx, int ny, int nz):_nx(nx),_ny(ny),_nz(nz){
		_bbox = bb;
		opaque.resize(nx*ny*nz); //Array
	}
	~Grid(){}

	virtual bool intersect(const Ray &r, Hit &h, float tmin);
	void initializeRayMarch(MarchingInfo &mi, const Ray &r, float tmin);
	bool intersectShadowRay(const Ray &r, Hit &hit, float tmin);
	virtual void paint();

};

// Group
class Group :public Object3D {
public:
	int _n; // num of object3d
	vector<Object3D*> objArray;
	Group(const int num);
	void addObject(int index, Object3D *obj);
	bool intersect(const Ray &r, Hit &h, float tmin);
	bool intersectShadowRay(const Ray &r, Hit &h, float tmin);
	void insertIntoGrid(Grid *g, Matrix *m);
	void paint(void);
};

// Sphere
class Sphere : public Object3D {
private:
	Vec3f _center;
	float _radius;
public:
	Sphere(Vec3f &center, float &radius, Material* material);
	bool intersect(const Ray &r, Hit &h, float tmin);
	void insertIntoGrid(Grid *g, Matrix *m);
	void paint(void);
};

// Plane
class Plane :public Object3D {
public:
	Vec3f _v_norm;
	float _d;
public:
	Plane(Vec3f &normal, float d, Material *m);
	Plane(Vec3f &p0, Vec3f &p1, Vec3f &p2, Material *m);
	BoundingBox *getBoundingBox() { return nullptr; }
	bool intersect(const Ray &r, Hit &h, float tmin);
	void insertIntoGrid(Grid *g, Matrix *m);
	void paint(void);
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
	bool is_triangle() { return true; }
	//三角形并不是变换对称的，变换后的bbox可能会改变，因此要进行额外处理?
	//TODO: 但是这个区别在哪里呢？
	BoundingBox *getTriangleBoundingBox(Matrix *m);//use a special case for transformed triangles
	void insertIntoGrid(Grid *g, Matrix *m);
	void paint(void);
};

//Transform
class Transform :public Object3D {
private:
	Object3D *_obj;
	Matrix _mat;
public:
	Transform(Matrix &mat, Object3D *o);
	bool intersect(const Ray &r, Hit &h, float tmin);
	BoundingBox *getBoundingBox();
	void insertIntoGrid(Grid *g, Matrix *m);
	void paint(void);
};



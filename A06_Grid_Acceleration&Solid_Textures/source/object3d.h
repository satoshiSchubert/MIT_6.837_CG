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

//������Ҫ��ǰ����һ��Grid
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
	vector<vector<Object3D*>> opaque;//ά��һ��vector�����vector�е�ĳһ��������ڶ��primitives�����Ի�Ҫ��һ��vector
	set<Object3D*> is_intersected;
	vector<Object3D*> infinitePrimitives;
	//std::set��Ϊ��׼���һ������������ʵ���ڲ�Ԫ�ؽ���������ʹ�������Կ��Զ�һ��Ԫ�ؽ��в�������
	//std::set���������������ѧ�С����ϡ��ĸ�����ṩ�Ľӿ�Ҳ����ˡ�

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
	//�����β����Ǳ任�ԳƵģ��任���bbox���ܻ�ı䣬���Ҫ���ж��⴦��?
	//TODO: ������������������أ�
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



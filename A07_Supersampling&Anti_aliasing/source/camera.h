#pragma once

#include "ray.h"
#include "vectors.h"
#include "matrix.h"


// Basic Class: Camera
class Camera {
public:
	virtual Ray generateRay(Vec2f point) = 0;
	virtual float getTMin() = 0;
	virtual void glInit(int w, int h) = 0;
	virtual void glPlaceCamera(void) = 0;
	virtual void dollyCamera(float dist) = 0;
	virtual void truckCamera(float dx, float dy) = 0;
	virtual void rotateCamera(float rx, float ry) = 0;
};


// Class OrthographicCamera
class OrthographicCamera :public Camera {
private:
	float _img_size;
	Vec3f _center;
	Vec3f _v_dir;
	Vec3f _v_up;
	Vec3f _v_up_copy;
	Vec3f _v_hori;

public:
	OrthographicCamera(Vec3f center, Vec3f v_dir, Vec3f v_up, float size);
	Ray generateRay(Vec2f point);
	float getTMin();
	void glInit(int w, int h);
	void glPlaceCamera(void);
	void dollyCamera(float dist);
	void truckCamera(float dx, float dy);
	void rotateCamera(float rx, float ry);
};


// Class PerspectiveCamera
class PerspectiveCamera :public Camera {
private:
	float _img_size;
	float _angle;
	Vec3f _center;
	Vec3f _v_dir;
	Vec3f _v_up;
	Vec3f _v_up_copy;
	Vec3f _v_hori;

public:
	PerspectiveCamera(Vec3f center, Vec3f v_dir, Vec3f v_up, float angle);
	Ray generateRay(Vec2f point);
	float getTMin();

	void glInit(int w, int h);

	void glPlaceCamera(void);

	void dollyCamera(float dist);

	void truckCamera(float dx, float dy);

	void rotateCamera(float rx, float ry);

};











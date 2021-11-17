#pragma once

#include "ray.h"
#include "vectors.h"

class Camera {
public:
	virtual Ray generateRay(Vec2f point) = 0;
	virtual float getTMin() = 0;
};

class OrthographicCamera :public Camera {
private:
	float _img_size;
	Vec3f _center;
	Vec3f _v_dir;
	Vec3f _v_up;
	Vec3f _v_hori;

public:
	OrthographicCamera(Vec3f center, Vec3f v_dir, Vec3f v_up, float size);
	Ray generateRay(Vec2f point);
	float getTMin();
};

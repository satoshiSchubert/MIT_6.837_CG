#pragma once
#include "camera.h"

class OrthographicCamera :Camera {

private:
	float _img_size;
	Vec3f _center;
	Vec3f _v_dir;
	Vec3f _v_up;
	Vec3f _v_hori;


public:
	OrthographicCamera(Vec3f center, Vec3f v_dir, Vec3f v_up, float size);
	Ray generatedRay(Vec2f point);
	float getTMin();
};
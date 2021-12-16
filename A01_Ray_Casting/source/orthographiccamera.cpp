#include "orthographiccamera.h"

using namespace std;

OrthographicCamera::OrthographicCamera(Vec3f center, Vec3f v_dir, Vec3f v_up, float size) {
	// 先根据up和direction求叉积算horizontal（hori和dir正交）
	//然后再根据这两个正交的向量算up
	v_dir.Normalize();
	Vec3f::Cross3(_v_hori, v_dir, v_up);
	_v_hori.Normalize();
	Vec3f::Cross3(v_up, _v_hori, v_dir);
	v_up.Normalize();

	_center = center;
	_v_dir = v_dir;
	_v_up = v_up;
	_img_size = size;
}


Ray OrthographicCamera::generatedRay(Vec2f point) {
	auto origin = _center + (point.x() - 0.5)*_img_size*_v_hori + (point.y() - 0.5)*_img_size*_v_up;
	Ray ray_gen(origin,_v_dir);
	return ray_gen;
}

float OrthographicCamera::getTMin() {
	return FLT_MIN;
}












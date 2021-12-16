#include "camera.h"

using namespace std;

// OrthographicCamera
OrthographicCamera::OrthographicCamera(Vec3f center, Vec3f v_dir, Vec3f v_up, float size) {
	// �ȸ���up��direction������horizontal��hori��dir������
	//Ȼ���ٸ���������������������up
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

Ray OrthographicCamera::generateRay(Vec2f point) {
	Vec3f origin = _center + (point.x() - 0.5)*_img_size*_v_hori + (point.y() - 0.5)*_img_size*_v_up;
	return Ray(origin, _v_dir);
}

float OrthographicCamera::getTMin() {
	return FLT_MIN;
}

// PerspectiveCamera
PerspectiveCamera::PerspectiveCamera(Vec3f center, Vec3f v_dir, Vec3f v_up, float angle) {
	// �ȸ���up��direction������horizontal��hori��dir������
	//Ȼ���ٸ���������������������up
	v_dir.Normalize();
	Vec3f::Cross3(_v_hori, v_dir, v_up);
	_v_hori.Normalize();
	Vec3f::Cross3(v_up, _v_hori, v_dir);
	v_up.Normalize();

	_center = center;
	_v_dir = v_dir;
	_v_up = v_up;
	// �����������Ļ�ľ���Ϊ1��λ����
	// �򵥵����Ǻ������㣬�ã�
	_img_size = 2.f * tanf(angle / 2.f);
}

Ray PerspectiveCamera::generateRay(Vec2f point) {
	Vec3f target = _v_dir + (point.x() - 0.5)*_img_size*_v_hori + (point.y() - 0.5)*_img_size*_v_up;
	target.Normalize();
	return Ray(_center, target);
}

float PerspectiveCamera::getTMin() {
	return 0; // TODO:may need to modify
}












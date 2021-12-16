#define GLUT_DISABLE_ATEXIT_HACK
#include <GL/freeglut.h>
#include "camera.h"


using namespace std;

// ========================================
// OrthographicCamera
// ========================================
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
	_v_up_copy = _v_up;
	_img_size = size;
}

Ray OrthographicCamera::generateRay(Vec2f point) {
	Vec3f origin = _center + (point.x() - 0.5)*_img_size*_v_hori + (point.y() - 0.5)*_img_size*_v_up;
	return Ray(origin, _v_dir);
}

float OrthographicCamera::getTMin() {
	return FLT_MIN;
}

void OrthographicCamera::glInit(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w > h)
		glOrtho(-_img_size / 2.0, _img_size / 2.0, -_img_size * (float)h / (float)w / 2.0, _img_size*(float)h / (float)w / 2.0, 0.5, 40.0);
	else
		glOrtho(-_img_size * (float)w / (float)h / 2.0, _img_size*(float)w / (float)h / 2.0, -_img_size / 2.0, _img_size / 2.0, 0.5, 40.0);
}

void OrthographicCamera::glPlaceCamera(void)
{
	gluLookAt(_center.x(), _center.y(), _center.z(),
		_center.x() + _v_dir.x(), _center.y() + _v_dir.y(), _center.z() + _v_dir.z(),
		_v_up.x(), _v_up.y(), _v_up.z());
}

// dollyCamera: Move camera along the direction vector
void OrthographicCamera::dollyCamera(float dist)
{
	_center += _v_dir * dist;
}

// truckCamera: Translate camera perpendicular to the direction vector
void OrthographicCamera::truckCamera(float dx, float dy)
{
	Vec3f horizontal;
	Vec3f::Cross3(horizontal, _v_dir, _v_up);
	horizontal.Normalize();

	Vec3f screenUp;
	Vec3f::Cross3(screenUp, horizontal, _v_dir);

	_center += horizontal * dx + screenUp * dy;
}

// rotateCamera: Rotate around the up and horizontal vectors
void OrthographicCamera::rotateCamera(float rx, float ry)
{
	Vec3f horizontal;
	Vec3f::Cross3(horizontal, _v_dir, _v_up);
	horizontal.Normalize();

	// Don't let the model flip upside-down (There is a singularity
	// at the poles when 'up' and 'direction' are aligned)
	float tiltAngle = acos(_v_up.Dot3(_v_dir));
	if (tiltAngle - ry > 3.13)
		ry = tiltAngle - 3.13;
	else if (tiltAngle - ry < 0.01)
		ry = tiltAngle - 0.01;

	Matrix rotMat = Matrix::MakeAxisRotation(_v_up, rx);
	rotMat *= Matrix::MakeAxisRotation(horizontal, ry);

	rotMat.Transform(_center);
	rotMat.TransformDirection(_v_dir);

}


// ========================================
// PerspectiveCamera
// ========================================
PerspectiveCamera::PerspectiveCamera(Vec3f center, Vec3f v_dir, Vec3f v_up, float angle) {
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
	_v_up_copy = _v_up;
	_angle = angle;
	// 假设相机到屏幕的距离为1单位长度
	// 简单的三角函数计算，得：
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

// Create a perspective camera with the appropriate dimensions that
// crops or stretches in the x-dimension as necessary
void PerspectiveCamera::glInit(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(_angle*180.0 / 3.14159, (float)w / float(h), 0.5, 40.0);
}

// Place a perspective camera within an OpenGL scene
void PerspectiveCamera::glPlaceCamera(void)
{
	gluLookAt(_center.x(), _center.y(), _center.z(),
		_center.x() + _v_dir.x(), _center.y() + _v_dir.y(), _center.z() + _v_dir.z(),
		_v_up.x(), _v_up.y(), _v_up.z());
}

// dollyCamera: Move camera along the direction vector
void PerspectiveCamera::dollyCamera(float dist)
{
	_center += _v_dir * dist;
}

void PerspectiveCamera::truckCamera(float dx, float dy)
{
	Vec3f horizontal;
	Vec3f::Cross3(horizontal, _v_dir,_v_up);
	horizontal.Normalize();

	Vec3f screenUp;
	Vec3f::Cross3(screenUp, horizontal, _v_dir);

	_center += horizontal * dx + screenUp * dy;
}

// rotateCamera: Rotate around the up and horizontal vectors
void PerspectiveCamera::rotateCamera(float rx, float ry)
{
	Vec3f horizontal;
	Vec3f::Cross3(horizontal, _v_dir, _v_up);
	horizontal.Normalize();

	// Don't let the model flip upside-down (There is a singularity
	// at the poles when 'up' and 'direction' are aligned)
	float tiltAngle = acos(_v_up.Dot3(_v_dir));
	if (tiltAngle - ry > 3.13)
		ry = tiltAngle - 3.13;
	else if (tiltAngle - ry < 0.01)
		ry = tiltAngle - 0.01;

	Matrix rotMat = Matrix::MakeAxisRotation(_v_up, rx);
	rotMat *= Matrix::MakeAxisRotation(horizontal, ry);

	rotMat.Transform(_center);
	rotMat.TransformDirection(_v_dir);
	_v_dir.Normalize();
}










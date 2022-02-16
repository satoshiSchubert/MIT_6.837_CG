#include "material.h"
#include <math.h>
#include <algorithm>
#include "glCanvas.h" 

#define GLUT_DISABLE_ATEXIT_HACK
#include <GL/freeglut.h>

#ifdef SPECULAR_FIX
// OPTIONAL:  global variable allows (hacky) communication 
// with glCanvas::display
extern int SPECULAR_FIX_WHICH_PASS;
#endif

Vec3f PhongMaterial::Shade(const Ray &ray, const Hit &hit, const Vec3f &dirToLight, const Vec3f &lightColor) {
	Vec3f pixel_color(0.f,0.f,0.f);
	Vec3f n = hit.getNormal();
	Vec3f v = -1 * ray.getDirection();
	v.Normalize();
	// diffuse
	pixel_color += getDiffuseColor() * lightColor * max(0.f, n.Dot3(dirToLight));
	// Blinn-Torrance version of Phong model
	// r2 is ignored
	Vec3f h = v + dirToLight;
	h.Normalize();//h.Divide(h.Length(), h.Length(), h.Length());
	pixel_color += _specularColor * lightColor * powf(max(0.f,n.Dot3(h)), _exponent);
	return pixel_color;
}


bool PhongMaterial::reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) {
	// 这个函数用来计算反射光线并传递给参数ray
	// reflectiveColor是材料的“反射率”，如果太小则认为这个点无法产生反射，返回false
	// 这个材料的“反射率”作为参数传给attenuation，以用来衰减光线的weight
	if (_reflectiveColor.Length() < 0.001)
		return false;
	// 计算反射光线
	// R=V-2(V·N)N | R:Reflected, V:View, N:Normal
	Vec3f ray_in = ray.getDirection();
	Vec3f normal = hit.getNormal();
	Vec3f ray_out = ray_in - 2.f*normal.Dot3(ray_in)*normal;
	ray_out.Normalize();
	reflected = Ray(hit.getIntersectionPoint(), ray_out);
	attenuation = _reflectiveColor;
	return true;
}


bool PhongMaterial::refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) {
	if (_transparentColor.Length() < 0.001)
		return false;
	Vec3f ray_in = ray.getDirection();
	Vec3f normal = hit.getNormal();
	// Snell-Descartes Law:nr = (sinT)/(sinI) = ni/nt
	float nr;
	if (ray_in.Dot3(normal) > 0) {
		normal = -1 * normal;
		nr = _indexOfRefraction;
	}
	else {
		// 正常情况
		nr = 1.f / _indexOfRefraction;// 空气是1
	}
	Vec3f I = ray_in * -1; // 参考示意图！
	float N_I = normal.Dot3(I);
	float t = 1 - pow(nr, 2)*(1 - pow(N_I, 2));
	// t should be greater than 0
	if (t > 0) {
		Vec3f T = (N_I * nr - sqrtf(t)) * normal - nr * I;
		T.Normalize();
		refracted = Ray(hit.getIntersectionPoint(), T);
		// 向外传递参数
		attenuation = _transparentColor;
		return true;
	}
	else {
		return false;
	}
}


void PhongMaterial::glSetMaterial(void) const {

	GLfloat one[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat zero[4] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat specular[4] = {
	  getSpecularColor().r(),
	  getSpecularColor().g(),
	  getSpecularColor().b(),
	  1.0 };
	GLfloat diffuse[4] = {
	  getDiffuseColor().r(),
	  getDiffuseColor().g(),
	  getDiffuseColor().b(),
	  1.0 };

	// NOTE: GL uses the Blinn Torrance version of Phong...      
	float glexponent = _exponent;
	if (glexponent < 0) glexponent = 0;
	if (glexponent > 128) glexponent = 128;

#if !SPECULAR_FIX 

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);

#else

	// OPTIONAL: 3 pass rendering to fix the specular highlight 
	// artifact for small specular exponents (wide specular lobe)

	if (SPECULAR_FIX_WHICH_PASS == 0) {
		// First pass, draw only the specular highlights
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, zero);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);

	}
	else if (SPECULAR_FIX_WHICH_PASS == 1) {
		// Second pass, compute normal dot light 
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	}
	else {
		// Third pass, add ambient & diffuse terms
		assert(SPECULAR_FIX_WHICH_PASS == 2);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	}

#endif
}

float T = 0.4; //周期，值越大则棋盘格间距越大
Vec3f CheckerBoard::Shade(const Ray &ray, const Hit &hit, const Vec3f &l, const Vec3f &lightColor) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);//世界坐标->Texture坐标
	//关键在于如何周期性实现棋盘格，一提到周期性可以联想到三角函数，那么我们可以通过一个很简单的方式来实现棋盘格相邻交错
	//对于XYZ三轴，将其对应的每轴的值的sine相乘，得到一个bool函数，这个函数的正负代表棋盘格的两色。
	float flg_sin = sin((1.f/T) * p.x()) * sin((1.f / T) * p.y()) * sin((1.f / T) * p.z());
	if (flg_sin > 0)
		return _material1->Shade(ray, hit, l, lightColor);
	else
		return _material2->Shade(ray, hit, l, lightColor);
}

void CheckerBoard::glSetMaterial() const {
	_material1->glSetMaterial();
}

bool CheckerBoard::reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);
	float flg_sin = sin((1.f / T) * p.x()) * sin((1.f / T) * p.y()) * sin((1.f / T) * p.z());
	if (flg_sin > 0)
		return _material1->reflect(ray, hit, attenuation, reflected);
	else
		return _material2->reflect(ray, hit, attenuation, reflected);
}

bool CheckerBoard::refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);
	float flg_sin = sin((1.f / T) * p.x()) * sin((1.f / T) * p.y()) * sin((1.f / T) * p.z());
	if (flg_sin > 0)
		return _material1->refract(ray, hit, attenuation, refracted);
	else
		return _material2->refract(ray, hit, attenuation, refracted);
}



Vec3f Noise::Shade(const Ray &ray, const Hit &hit, const Vec3f &l, const Vec3f &lightColor) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);

	float N = 0.f;
	for (int i = 0; i < octaves; i++) {
		int I = pow(2, i);
		N += PerlinNoise::noise(I*p.x(), I*p.y(), I*p.z()) / (1.f*I);
	}
	//Rescale
	if (N > 1) N = 1;
	if (N < 0) N = 0;
	//差值
	return _material1->Shade(ray, hit, l, lightColor) * N + (1-N) * _material2->Shade(ray, hit, l, lightColor);
}

void Noise::glSetMaterial() const {}


bool Noise::reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);

	float N = 0.f;
	for (int i = 0; i < octaves; i++) {
		int I = pow(2, i);
		N += PerlinNoise::noise(I*p.x(), I*p.y(), I*p.z()) / (1.f*I);
	}
	//Rescale
	if (N > 1) N = 1;
	if (N < 0) N = 0;

	if (_material1->reflect(ray, hit, attenuation, reflected)) {
		//attenuation是衰减率
		//这里取两种材料底色的差值？
		attenuation = _material1->getDiffuseColor() * N + (1 - N) * _material2->getDiffuseColor();
		return true;
	}
	return false;
}

bool Noise::refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);

	float N = 0.f;
	for (int i = 0; i < octaves; i++) {
		int I = pow(2, i);
		N += PerlinNoise::noise(I*p.x(), I*p.y(), I*p.z()) / (1.f*I);
	}
	//Rescale
	if (N > 1) N = 1;
	if (N < 0) N = 0;

	if (_material1->refract(ray, hit, attenuation, refracted)) {
		//attenuation是衰减率
		//这里取两种材料底色的差值？
		attenuation = _material1->getDiffuseColor() * N + (1 - N) * _material2->getDiffuseColor();
		return true;
	}
	return false;
}

Vec3f Noise::getDiffuseColor() const {
	return (_material1->getDiffuseColor() + _material2->getDiffuseColor()) * 0.5;
}


Vec3f Marble::Shade(const Ray &ray, const Hit &hit, const Vec3f &l, const Vec3f &lightColor) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);

	float N = 0.f;
	for (int i = 0; i < octaves; i++) {
		int I = pow(2, i);
		N += PerlinNoise::noise(I*p.x(), I*p.y(), I*p.z()) / (1.f*I);
	}

	float M = sin(frequency * p.x() + amplitude * N);
	//Rescale
	if (M > 1) M = 1;
	if (M < 0) M = 0;
	//差值
	return _material1->Shade(ray, hit, l, lightColor) * M + (1 - M) * _material2->Shade(ray, hit, l, lightColor);
}

Vec3f Marble::getDiffuseColor() const {
	return (_material1->getDiffuseColor() + _material2->getDiffuseColor()) * 0.5;
}

void Marble::glSetMaterial() const {}

bool Marble::reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);

	float N = 0.f;
	for (int i = 0; i < octaves; i++) {
		int I = pow(2, i);
		N += PerlinNoise::noise(I*p.x(), I*p.y(), I*p.z()) / (1.f*I);
	}
	float M = sin(frequency * p.x() + amplitude * N);
	//Rescale
	if (M > 1) M = 1;
	if (M < 0) M = 0;

	if (_material1->reflect(ray, hit, attenuation, reflected)) {
		//attenuation是衰减率
		//这里取两种材料底色的差值？
		attenuation = _material1->getDiffuseColor() * M + (1 - M) * _material2->getDiffuseColor();
		return true;
	}
	return false;
}

bool Marble::refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);

	float N = 0.f;
	for (int i = 0; i < octaves; i++) {
		int I = pow(2, i);
		N += PerlinNoise::noise(I*p.x(), I*p.y(), I*p.z()) / (1.f*I);
	}
	float M = sin(frequency * p.x() + amplitude * N);
	//Rescale
	if (M > 1) M = 1;
	if (M < 0) M = 0;

	if (_material1->refract(ray, hit, attenuation, refracted)) {
		//attenuation是衰减率
		//这里取两种材料底色的差值？
		attenuation = _material1->getDiffuseColor() * M + (1 - M) * _material2->getDiffuseColor();
		return true;
	}
	return false;
}

// >>>>>>>>>>>>Wood类还没施工>>>>>>>>>>>>>>
Vec3f Wood::Shade(const Ray &ray, const Hit &hit, const Vec3f &l, const Vec3f &lightColor) {
	Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);

	float N = 0.f;
	for (int i = 0; i < octaves; i++) {
		int I = pow(2, i);
		N += PerlinNoise::noise(I*p.x(), I*p.y(), I*p.z()) / (1.f*I);
	}

	float M = sin(frequency * p.x() + amplitude * N);
	//Rescale
	if (M > 1) M = 1;
	if (M < 0) M = 0;
	//差值
	return _material1->Shade(ray, hit, l, lightColor) * M + (1 - M) * _material2->Shade(ray, hit, l, lightColor);
}

void Wood::glSetMaterial() const {}

bool Wood::reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) {
	return false;
}
bool Wood::refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) {
	return false;
}

Vec3f Wood::getDiffuseColor() const {
	return (_material1->getDiffuseColor() + _material2->getDiffuseColor()) * 0.5;
}














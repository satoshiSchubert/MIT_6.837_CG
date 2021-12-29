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





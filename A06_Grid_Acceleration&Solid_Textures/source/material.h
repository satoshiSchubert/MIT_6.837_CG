#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vectors.h"
#include "ray.h"
#include "hit.h"
#include "matrix.h"
#include "perlin_noise.h"

// ====================================================================
// ====================================================================

// You will extend this class in later assignments

class Material {

public:

	// CONSTRUCTORS & DESTRUCTOR
	Material(const Vec3f& d_color) { _diffuseColor = d_color; }
	Material() {}
	virtual ~Material() {};

	// ACCESSORS
	virtual Vec3f getDiffuseColor() const { return _diffuseColor; }

	// VIRTUAL FUNC
	virtual Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &dirToLight, const Vec3f &lightColor) = 0;
	virtual void glSetMaterial(void) const = 0;
	virtual bool reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) = 0;
	virtual bool refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) = 0;

protected:

	// REPRESENTATION
	Vec3f _diffuseColor;

};

class PhongMaterial : public Material {

protected:
	Vec3f _specularColor;
	float _exponent = 0;
	Vec3f _reflectiveColor;
	Vec3f _transparentColor;
	float _indexOfRefraction = 0;

public:
	PhongMaterial(const Vec3f &diffuseColor) :Material(diffuseColor) {}

	PhongMaterial(const Vec3f &diffuseColor,
		const Vec3f &specularColor, 
		float exponent,
		const Vec3f &reflectiveColor,
		const Vec3f &transparentColor,
		float indexOfRefraction):Material(diffuseColor) {
		_specularColor = specularColor;
		_exponent = exponent;
		_reflectiveColor = reflectiveColor;
		_transparentColor = transparentColor;
		_indexOfRefraction = indexOfRefraction;// 材料的折射指数（v2）
		// v21 = v1/v2 , 默认真空，v1=1
	}

	Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &dirToLight, const Vec3f &lightColor);
	void glSetMaterial(void) const;
	bool reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected);
	bool refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted);
	
	//Accessors
	Vec3f getSpecularColor() const { return _specularColor; }
	Vec3f getReflectiveColor() const { return _reflectiveColor; }
	Vec3f getTransparentColor() const { return _transparentColor; }

};

class CheckerBoard :public Material {
public:
	Material* _material1;
	Material* _material2;
	Matrix *_mat_wd2tex;

public:
	CheckerBoard(Matrix *m, Material *mat1, Material *mat2):Material() {
		_material1 = mat1;
		_material2 = mat2;
		_mat_wd2tex = m;
	}

	Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &l, const Vec3f &lightColor) override;
	void glSetMaterial() const override;
	bool reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) override;
	bool refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) override;

};

class Noise : public Material {
public:
	Material *_material1;
	Material *_material2;
	Matrix *_mat_wd2tex;
	int octaves;

public:
	Noise(Matrix *m, Material *mat1, Material *mat2, int octaves) : 
		Material(), 
		_mat_wd2tex(m), 
		_material1(mat1), 
		_material2(mat2),
		octaves(octaves) {}

	Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &l, const Vec3f &lightColor) override;
	void glSetMaterial()const override;
	bool reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) override;
	bool refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) override;
	Vec3f getDiffuseColor()const override;
};


class Marble : public Material {
public:
	Material *_material1;
	Material *_material2;
	Matrix *_mat_wd2tex;
	int octaves;
	float frequency;
	float amplitude;

public:
	Marble(Matrix *m, Material *mat1, Material *mat2, int octaves, float frequency, float amplitude) :
		Material(Vec3f()), 
		_mat_wd2tex(m),
		_material1(mat1), 
		_material2(mat2), 
		octaves(octaves), 
		frequency(frequency), 
		amplitude(amplitude){}

	Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &l, const Vec3f &lightColor) override;
	void glSetMaterial() const override;
	bool reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) override;
	bool refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) override;
	Vec3f getDiffuseColor() const override;
};


class Wood : public Material {
private:
	Material *_material1;
	Material *_material2;
	Matrix *_mat_wd2tex;
	int octaves;
	float frequency;
	float amplitude;

public:
	Wood(Matrix *m, Material *mat1, Material *mat2, int octaves, float frequency, float amplitude) :
		Material(Vec3f()), 
		_mat_wd2tex(m),
		_material1(mat1), 
		_material2(mat2), 
		octaves(octaves), 
		frequency(frequency), 
		amplitude(amplitude) {}

	Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &l, const Vec3f &lightColor) override;
	void glSetMaterial() const override;
	bool reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) override;
	bool refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) override;
	Vec3f getDiffuseColor() const override;

};

















// ====================================================================
// ====================================================================

#endif
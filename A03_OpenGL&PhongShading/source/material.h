#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vectors.h"
#include "ray.h"
#include "hit.h"

// ====================================================================
// ====================================================================

// You will extend this class in later assignments

class Material {

public:

	// CONSTRUCTORS & DESTRUCTOR
	Material(const Vec3f& d_color) { _diffuseColor = d_color; }
	virtual ~Material() {};

	// ACCESSORS
	virtual Vec3f getDiffuseColor() const { return _diffuseColor; }

	// VIRTUAL FUNC
	virtual Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &dirToLight, const Vec3f &lightColor) = 0;
	virtual void glSetMaterial(void) const = 0;

protected:

	// REPRESENTATION
	Vec3f _diffuseColor;

};

class PhongMaterial : public Material {

protected:
	Vec3f _specularColor;
	float _exponent;

public:
	PhongMaterial(const Vec3f &diffuseColor, const Vec3f &specularColor, float exponent):Material(diffuseColor) {
		_specularColor = specularColor;
		_exponent = exponent;
	}
	Vec3f Shade(const Ray &ray, const Hit &hit, const Vec3f &dirToLight, const Vec3f &lightColor);
	Vec3f getSpecularColor() const { return _specularColor; }
	void glSetMaterial(void) const;

};



// ====================================================================
// ====================================================================

#endif
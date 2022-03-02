#pragma once
#include <math.h>
#include <vector>
#include<iostream>
#include "matrix.h"
#include "vectors.h"
#include "random.h"

class ForceField {
public:
	Random *random;
public:
	
	ForceField(){}

	virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const = 0;

};

class GravityForceField :public ForceField {
private:
	Vec3f gravity;

public:
	GravityForceField(Vec3f gravity) :gravity(gravity) {	}

	virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;
};

class ConstantForceField : public ForceField {
private:
	Vec3f force;

public:
	ConstantForceField(Vec3f force) :force(force) {	}

	virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;
};

class RadialForceField : public ForceField {
private:
	float magnitude;

public:
	RadialForceField(float magnitude):magnitude(magnitude){}

	virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;
};

class VerticalForceField : public ForceField {
private:
	float magnitude;

public:
	VerticalForceField(float magnitude) :magnitude(magnitude){}

	virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t) const;
};

class WindForceField : public ForceField {
private:
	float magnitude;

public:
	WindForceField(float magnitude) :magnitude(magnitude) {}

	virtual Vec3f getAcceleration(const Vec3f &position, float mass, float t)const;
};




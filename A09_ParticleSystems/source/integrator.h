#pragma once
#include <math.h>
#include <vector>
#include<iostream>
#include "matrix.h"
#include "vectors.h"
#include "forcefield.h"
#include "particle.h"

class Integrator{
private:

public:
	Particle *ptc = nullptr;

	Integrator(){}

	virtual void Update(Particle *particle, ForceField *forcefield, float t, float dt) = 0;

	virtual Vec3f getColor()=0;
};

class EulerIntegrator :public Integrator {
private:

public:
	EulerIntegrator(){}

	virtual void Update(Particle *particle, ForceField *forcefield, float t, float dt);

	Vec3f getColor() override;
};

class MidpointIntegrator :public Integrator {
private:

public:
	MidpointIntegrator() {}

	virtual void Update(Particle *particle, ForceField *forcefield, float t, float dt);

	Vec3f getColor() override;
};


class RungeKuttaIntegrator :public Integrator {
private:

public:
	RungeKuttaIntegrator() {}

	virtual void Update(Particle *particle, ForceField *forcefield, float t, float dt) {};

	Vec3f getColor() override {
		return Vec3f(0, 0, 0);
	}
};
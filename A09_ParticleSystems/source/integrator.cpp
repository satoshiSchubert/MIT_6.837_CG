
#include "integrator.h"

void EulerIntegrator::Update(Particle *particle, ForceField *forcefield, float t, float dt) {

	Vec3f p = particle->getPosition();
	Vec3f v = particle->getVelocity();
	float m = particle->getMass();
	Vec3f a = forcefield->getAcceleration(p, m, t);

	Vec3f p_new = p + v * dt;
	Vec3f v_new = v + a * dt;
	 
	particle->setPosition(p_new);
	particle->setVelocity(v_new);

	ptc = particle;
}

Vec3f EulerIntegrator::getColor() {
	if (ptc != nullptr)
		return ptc->getColor();
}

void MidpointIntegrator::Update(Particle *particle, ForceField *forcefield, float t, float dt) {

	Vec3f p = particle->getPosition();
	Vec3f v = particle->getVelocity();
	float m = particle->getMass();
	Vec3f a = forcefield->getAcceleration(p, m, t);

	Vec3f p_mid = p + v * (dt / 2.f);
	Vec3f v_mid = v + a * (dt / 2.f);
	Vec3f a_mid = forcefield->getAcceleration(p_mid, m, t + (dt / 2.f));

	Vec3f p_new = p + v_mid * dt;
	Vec3f v_new = v + a_mid * dt;

	particle->setPosition(p_new);
	particle->setVelocity(v_new);

	ptc = particle;
}

Vec3f MidpointIntegrator::getColor() {
	if (ptc != nullptr)
		return ptc->getColor();
}

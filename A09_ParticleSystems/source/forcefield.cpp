#include"forcefield.h"

Vec3f GravityForceField::getAcceleration(const Vec3f &position, float mass, float t)const {
	return this->gravity;
}

Vec3f ConstantForceField::getAcceleration(const Vec3f &position, float mass, float t)const {
	Vec3f a = force;
	a.Divide(mass, mass, mass);
	return a;
}

Vec3f RadialForceField::getAcceleration(const Vec3f &position, float mass, float t)const {
	//suppose the center is (0,0,0)?
	Vec3f dir = -1 * position;
	return (dir * magnitude) * (1.f/mass);
}

Vec3f VerticalForceField::getAcceleration(const Vec3f &position, float mass, float t)const {
	Vec3f a = magnitude * Vec3f(0.f, -1.f*position.y(), 0.f) * (1.f / mass);
	return a;
}

Vec3f WindForceField::getAcceleration(const Vec3f &position, float mass, float t) const {
	Vec3f force = magnitude * Vec3f(cos(12 * t)*random->next(), sin(12 * t)*random->next(), 0);
	return force * (1.f / mass);
}



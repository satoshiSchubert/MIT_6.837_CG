#include "generator.h"
#include <GL/freeglut.h>

void Generator::SetColors(Vec3f color, Vec3f dead_color, float color_randomness) {
	this->colors = color;
	this->dead_color = dead_color;
	this->color_rand = color_randomness;
}

void Generator::SetLifespan(float lifespan, float lifespan_randomness, int desired_num_particles) {
	this->lifespan = lifespan;
	this->lifespan_rand = lifespan_randomness;
	this->desired_n_particles = desired_num_particles;
}

void Generator::SetMass(float mass, float mass_randomness) {
	this->mass = mass;
	this->mass_rand = mass_randomness;
}

int HoseGenerator::numNewParticles(float current_time, float dt)  {
	return dt * this->desired_n_particles / lifespan;
}

Particle* HoseGenerator::Generate(float current_time, int i) {
	Vec3f p = this->position + position_rand * random->randomVector();
	Vec3f v = this->velocity + velocity_rand * random->randomVector();
	Vec3f c = this->colors + color_rand * random->randomVector();
	Vec3f dc = this->dead_color + color_rand * random->randomVector();
	float m = this->mass + mass_rand * random->next();
	float l = this->lifespan + lifespan_rand * random->next();
	Particle *ptc = new Particle(p, v, c, dc, m, l);
	return ptc;
}

void HoseGenerator::Paint() {
	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_QUADS);
	glVertex3f(5, -4, 5);
	glVertex3f(5, -4, -5);
	glVertex3f(-5, -4, -5);
	glVertex3f(-5, -4, 5);
	glEnd();
}

int RingGenerator::numNewParticles(float current_time, float dt) {
	return (current_time < 3 ? current_time / 3 : 1) * desired_n_particles; //这是什么原理？
}

Particle *RingGenerator::Generate(float current_time, int i) {
	float r = current_time < 3 ? current_time : 3;
	Vec3f p = Vec3f(r*cos(current_time) + 0.233*random->next(), -4, r*sin(current_time) + 0.233*random->next());
	Vec3f v = this->velocity + velocity_rand * random->randomVector();
	Vec3f c = this->colors + color_rand * random->randomVector();
	Vec3f dc = this->dead_color + color_rand * random->randomVector();
	float m = this->mass + mass_rand * random->next();
	float l = this->lifespan + lifespan_rand * random->next();
	Particle *ptc = new Particle(p, v, c, dc, m, l);
	return ptc;
}

void RingGenerator::Paint() {
	glColor3f(0.3, 0.3, 0.3);
	glBegin(GL_QUADS);
	glVertex3f(5, -4, 5);
	glVertex3f(5, -4, -5);
	glVertex3f(-5, -4, -5);
	glVertex3f(-5, -4, 5);
	glEnd();
}











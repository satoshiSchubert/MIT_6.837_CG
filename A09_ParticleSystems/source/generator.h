#pragma once
#include <math.h>
#include <vector>
#include<iostream>
#include "matrix.h"
#include "vectors.h"
#include "particle.h"
#include "random.h"

class Generator{
protected:
	Vec3f colors;
	Vec3f dead_color;
	float color_rand;
	float lifespan;
	float lifespan_rand;
	int desired_n_particles;
	float mass;
	float mass_rand;
	Random *random;

public:
	Generator(){}

	// initialization
	void SetColors(Vec3f color, Vec3f dead_color, float color_randomness);
	void SetLifespan(float lifespan, float lifespan_randomness, int desired_num_particles);
	void SetMass(float mass, float mass_randomness);

	// on each timestep, create some particles
	virtual int numNewParticles(float current_time, float dt)=0;
	virtual Particle* Generate(float current_time, int i)=0;

	// for the gui
	virtual void Paint()=0;
	virtual void Restart() { delete random; random = new Random; }

};

class HoseGenerator :public Generator{

private:
	Vec3f position;
	float position_rand;
	Vec3f velocity;
	float velocity_rand;

public:
	HoseGenerator(Vec3f position, float position_randomness, Vec3f velocity, float velocity_randomness) :
		position(position),
		position_rand(position_randomness),
		velocity(velocity),
		velocity_rand(velocity_randomness) {	}

	int numNewParticles(float current_time, float dt)override;

	Particle *Generate(float current_time, int i) override;

	void Paint()  override;
};

class RingGenerator : public Generator {
public:
	RingGenerator(float position_randomness, const Vec3f &velocity, float velocity_randomness) :
		position_rand(position_randomness), 
		velocity(velocity), 
		velocity_rand(velocity_randomness) {}

	int numNewParticles(float current_time, float dt)override;

	Particle *Generate(float current_time, int i) override;

	void Paint()  override;

private:
	float position_rand;
	Vec3f velocity;
	float velocity_rand;
};





















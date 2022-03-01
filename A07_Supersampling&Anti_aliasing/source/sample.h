#pragma once
#ifndef _SAMPLE_H_
#define _SAMPLE_H_
#include <iostream>
#include <cstdlib>
#include "vectors.h"
#include <vector>
const int N = 999;//随机数精度为0.001 

// ==================================================================
// A helper class for super-sampling.  The position specifies the 2D
// offset within the pixel from (0,0) -> (1,1).
//一个样本点，储存了某个像素的位置和颜色
class Sample {

public:

	// CONSTRUCTOR & DESTRUCTOR
	Sample() {
		position = Vec2f(0.5, 0.5);
		color = Vec3f(0, 0, 0);
	}
	~Sample() {}

	// ACCESSORS
	Vec2f getPosition() { return position; }
	Vec3f getColor() { return color; }

	// MODIFIERS
	void set(Vec2f p, Vec3f c) {
		if (!(p.x() >= 0 && p.x() <= 1)) {
			cout << p.x() << endl;
			assert(p.x() >= 0 && p.x() <= 1);
		}
		assert(p.x() >= 0 && p.x() <= 1);
		assert(p.y() >= 0 && p.y() <= 1);
		position = p;
		color = c;
	}

private:

	// REPRESENTATION
	Vec2f position;
	Vec3f color;

};


class Sampler{
public:
	int n_samples;
	vector<Sample> samples;

public:
	Sampler(int num_samples) :n_samples(num_samples){
		samples = vector<Sample>(n_samples);
	}

	// returns the 2D offset for the specified sample
	virtual Vec2f getSamplePosition(int n) {
		return Vec2f(0.f, 0.f);
	}

};


class RandomSampler :public Sampler {
private:
	float get_random_0to1() {
		return 1.f * rand()/RAND_MAX;
	}

public:
	RandomSampler(int num_samples) :Sampler(num_samples) {}

	virtual Vec2f getSamplePosition(int n) {
		return Vec2f(get_random_0to1(), get_random_0to1());
	}
	
};

class UniformSampler :public Sampler {
private:
	int size;
	float d;

public:
	UniformSampler(int num_samples) :Sampler(num_samples) {
		size = sqrt(num_samples);
		d = 1.f / (size + 1);
	}

	virtual Vec2f getSamplePosition(int n) {
		if (n_samples == 1)
			return Vec2f(0.f, 0.f);
		return Vec2f((n%size + 1)*d, (n/size + 1)*d);
	}
};

//在Uniform的基础上加扰动，比纯random更均匀
class JitteredSampler :public Sampler {
private:
	int size;
	float d;

public:
	JitteredSampler(int num_samples) :Sampler(num_samples) {
		size = sqrt(num_samples);
		d = 0.9999f / (size + 1);
	}

	virtual Vec2f getSamplePosition(int n) {
		if (n_samples == 1)
			return Vec2f(0.f, 0.f);
		return Vec2f(((n%size + 1)*d + ((2.f*rand() / RAND_MAX)*d - 1.f*d)), (n / size + 1)*d + ((2.f*rand() / RAND_MAX)*d - 1.f*d));
	}

};





// ==================================================================

#endif
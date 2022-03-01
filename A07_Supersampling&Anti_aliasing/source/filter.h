#pragma once
#include "film.h"

class Filter {
public:
	Filter(){}

	Vec3f getColor(int i, int j, Film *film);
	
	//returns the weight for point (x+pexel_center,y+pixel_center), and will have maximum in pixel_center(x=y=0)
	virtual float getWeight(float x, float y) = 0; 
	//we dont need to access all samples in the Film, just scan samples within the radius
	virtual int getSupportRadius() = 0;
};


class BoxFilter :public Filter {
private:
	float radius;
public:
	BoxFilter(float radius):radius(radius){}

	float getWeight(float x, float y) override;

	int getSupportRadius() override;
};


class TentFilter :public Filter {
private:
	float radius;
public:
	TentFilter(float radius) :radius(radius) {}

	float getWeight(float x, float y) override;

	int getSupportRadius() override;
};


class GaussianFilter :public Filter {
private:
	float radius;
public:
	GaussianFilter(float radius) :radius(radius) {}

	float getWeight(float x, float y) override;

	int getSupportRadius() override;
};





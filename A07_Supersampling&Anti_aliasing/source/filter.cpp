#include "filter.h"

Vec3f Filter::getColor(int i,int j, Film* film) {
	Vec3f color(0.f, 0.f, 0.f);
	float weightsum = 0;// 归一化
	int support_radius = getSupportRadius();
	int width = film->getWidth();
	int height = film->getHeight();
	
	for (int ii = i - support_radius; ii <= i + support_radius; ii++) {
		for (int jj = j - support_radius; jj <= j + support_radius; jj++) {
			if (ii >= width || ii<0 || jj>=height || jj < 0) continue;//越界处理
			for (int kk = 0; kk < film->getNumSamples(); kk++) {
				Sample s = film->getSample(ii, jj, kk);
				float xx = ii + s.getPosition().x() - (i + 0.5);
				float yy = jj + s.getPosition().y() - (j + 0.5);
				float weight = getWeight(xx, yy);
				color += s.getColor()*weight;
				weightsum += weight;
			}
		}
	}
	return color * (1.f / weightsum);
}


// BoxFilter: if pixel is inside radius,return 1,otherwise return 0.
float BoxFilter::getWeight(float x, float y) {
	
	if (fabs(x) > radius || fabs(y) > radius) {
		return 0;
	}
	return 1;
}

int BoxFilter::getSupportRadius() {
	return radius + 0.499;//四舍五入
}

// TentFilter: y = 1-x(y>0)
float TentFilter::getWeight(float x, float y) {
	float dist = sqrt(x*x + y * y);
	if (dist > radius) return 0;
	return 1 - dist / radius;
}

int TentFilter::getSupportRadius() {
	return radius + 0.499;
}


// Gaussian
float GaussianFilter::getWeight(float x, float y) {
	float d2 = x * x + y * y;
	float d = sqrt(d2);
	float sigma = 1.f*radius;
	if (d > 2 * sigma)
		return 0;
	float upper = (-1.f*d2) / (2.f*sigma*sigma);
	return pow(2.718281828, upper);
}

int GaussianFilter::getSupportRadius() {
	return radius * 2;
}







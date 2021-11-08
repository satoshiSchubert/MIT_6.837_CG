
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "ifs.h"

void IFS::input(const char* file) {
	FILE *input = fopen(file, "r");
	assert(input != nullptr);

	// read the number of transforms.
	int num_trans;
	fscanf(input, "%d", &num_trans);
	_nTrans = num_trans;

	//read in the transforms
	for (int i = 0; i < num_trans; i++) {
		float prob;
		fscanf(input, "%f", &prob);
		Matrix m;
		m.Read3x3(input);
		_matTrans.push_back(m);
		_probTrans.push_back(prob);
	}

	// remember to close the file
	fclose(input);
}

// NEED DEBUGGING 0.71
void IFS::render2Image(Image& image, int num_points, int num_iters) {
	// set color
	Vec3f c_white(1.f, 1.f, 1.f);
	Vec3f c_black(0.f, 0.f, 0.f);
	image.SetAllPixels(c_white);
	srand((unsigned)time(0));

	for (int i = 0; i < num_points; i++) {
		Vec2f curr(rand() *1.0f / (RAND_MAX + 1), rand()*1.0f / (RAND_MAX + 1));
		
		for (int j = 0; j < num_iters; j++) {
			float tranSelectProb = rand()*1.0f / (RAND_MAX+1);
			//BUG
			int indTrans = _probParser(tranSelectProb);
			
			_matTrans[indTrans].Transform(curr);
		}
		if (curr.x() < 0 || curr.x() > 1 || curr.y() < 0 || curr.y() > 1) continue;

		int _x = curr.x()*image.Width();
		int _y = curr.y()*image.Height();
		
		
		image.SetPixel(_x, _y, c_black);

	}

}


int IFS::_probParser(float prob) {
	// return the index of the transform to do by given probability;
	float probSum = 0;
	int i;
	for (i = 0; i < _probTrans.size(); i++) {
		probSum += _probTrans[i];
		//cout << "prob:" << prob << "probsum:" << probSum << endl;
		if (probSum >= prob) {
			break;
		}
	}
	assert(i < _matTrans.size());
	return i;
}




#include "matrix.h"
#include "image.h"
#include "vectors.h"
#include <vector>

class IFS {
private:
	int _nTrans;
	vector<Matrix> _matTrans;
	vector<float> _probTrans;

public:
	// CONSTRUCTOR AND DECONSTRUCTOR
	IFS() :_nTrans(0), _matTrans(NULL), _probTrans(NULL){}
	~IFS() {
		// 用new申请的内存，释放时用delete; new[]申请的内存，释放时用delete[]
		//https://blog.csdn.net/u012936940/article/details/80919880
	}
	// Read file into IFS.
	void input(const char* file);
	// Render.
	void render2Image(Image& image, int num_points, int num_iters);
	int _probParser(float prob);
};
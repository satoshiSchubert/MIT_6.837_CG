## A4. Shadows, Reflection & Refraction
---

### 记录点

1. 纹理生成
    
    这次的作业中都是程序性纹理，也就是没有使用贴图的，因此如何设计这个纹理的程序就很关键。

    对于最基本的棋盘格，关键在于如何周期性实现棋盘格，一提到周期性可以联想到三角函数，那么我们可以通过一个很简单的方式来实现棋盘格相邻交错。
    
    对于XYZ三轴，将其对应的每轴的值的sine相乘，得到一个bool函数，这个函数的正负代表棋盘格的两色。

    ```cpp
    float flg_sin = sin((1.f/T) * p.x()) * sin((1.f / T) * p.y()) * sin((1.f / T) * p.z());
	if (flg_sin > 0)
		return _material1->Shade(ray, hit, l, lightColor);
	else
		return _material2->Shade(ray, hit, l, lightColor);
    ```

2. 噪声生成

    使用Perlin噪声：

    ```cpp
    Vec3f p = hit.getIntersectionPoint();
	_mat_wd2tex->Transform(p);

	float N = 0.f;
	for (int i = 0; i < octaves; i++) {
		int I = pow(2, i);
		N += PerlinNoise::noise(I*p.x(), I*p.y(), I*p.z()) / (1.f*I);
	}
	//Rescale
	if (N > 1) N = 1;
	if (N < 0) N = 0;
    //然后用这里的N做差值，也就是N*A+(1-N)*B
    ```

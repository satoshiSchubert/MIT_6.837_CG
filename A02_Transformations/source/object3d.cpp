#include "object3d.h"
#include <math.h>
#include <algorithm>

using namespace std;

float det2(float a, float b,
	float c, float d) {
	return a * d - b * c;
}

float det3(float a1, float a2, float a3,
	float b1, float b2, float b3,
	float c1, float c2, float c3) {
	return
		a1 * det2(b2, b3, c2, c3)
		- b1 * det2(a2, a3, c2, c3)
		+ c1 * det2(a2, a3, b2, b3);
}


//>>>>>>>>>>SPHERE>>>>>>>>>>
Sphere::Sphere(Vec3f &center, float &radius, Material* material) :_center(center), _radius(radius) {
	// 子类的初始化列表不能初始化父类的成员
	//https://blog.csdn.net/yanlei208/article/details/101089966
	_material = material; //这里则是赋值初始化
}

bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
	Vec3f Ro = r.getOrigin() - _center;
	Vec3f Rd = r.getDirection();
	float b = 2 * Ro.Dot3(Rd);
	float a = Rd.Dot3(Rd);
	float c = Ro.Dot3(Ro) - _radius * _radius;
	float delta = b * b - 4 * a * c;
	if (delta < 0)
		// No hit.
		return false;
	float t;
	float t1 = (-b - sqrt(delta)) / (2 * a);
	float t2 = (-b + sqrt(delta)) / (2 * a);
	
	// Add normal vector calculation:
	// Hit point - center
	
	if (t1 >= tmin) {
		t = min(t1, t2);
	}
	else {
		t = t2;
	}

	if (t < h.getT() && t>=0) {//closer当前交点，更新
		Vec3f v_norm = Vec3f(r.pointAtParameter(t), _center);//HACK:增加法向量的计算
		//Vec3f v_norm = Ro + t * Rd;
		v_norm.Normalize();
		h = Hit(t, _material,v_norm);
	}
	return true;
}
//<<<<<<<<<<SPHERE<<<<<<<<<<



//>>>>>>>>>>GROUP>>>>>>>>>>
Group::Group(const int num) :_n(num) {};

void Group::addObject(int index, Object3D *obj) {
	assert(index < _n);
	objArray.push_back(obj);
}

bool Group::intersect(const Ray &r, Hit &h, float tmin) {
	bool success = false;
	for (auto obj : objArray) {
		if (obj->intersect(r, h, tmin)) {
			success = true;
		}
	}
	return success;
}
//<<<<<<<<<<GROUP<<<<<<<<<<<



//>>>>>>>>>>PLANE>>>>>>>>>>
Plane::Plane(Vec3f &normal, float d, Material *m) {
	_v_norm = normal;
	_v_norm.Normalize();
	_material = m;
	_d = d;
}

Plane::Plane(Vec3f &p0, Vec3f &p1, Vec3f &p2, Material *m) {
	Vec3f::Cross3(_v_norm, p1 - p0, p2 - p1);
	_v_norm.Normalize();
	_d = _v_norm.Dot3(p0);
	_material = m;
}

bool Plane::intersect(const Ray &r, Hit &h, float tmin) {
	// 注意：Vec3f点乘时，左边应是浮点数，否则可能被强制转换成0
	Vec3f Ro = r.getOrigin();
	Vec3f Rd = r.getDirection();
	float denom = _v_norm.Dot3(Rd);
	if (denom == 0) return false;//分母不能为0
	float t = (_d - _v_norm.Dot3(Rd)) / denom;
	if (t > tmin && t < h.getT()) {
		h.set(t, _material, _v_norm,r);
		return true;
	}
	return false;
}
//<<<<<<<<<<PLANE<<<<<<<<<<



//>>>>>>>>>>TRIANGLE>>>>>>>>>>
Triangle::Triangle(Vec3f &a, Vec3f &b, Vec3f &c, Material *m) {
	_pa = a;
	_pb = b;
	_pc = c;
	_material = m;
	Vec3f::Cross3(_v_norm, _pb-_pa, _pc-_pa); //ab向量和ac向量
	_v_norm.Normalize();

}

bool Triangle::intersect(const Ray &r, Hit &h, float tmin) {
	//这里偷懒了，参考的别人的代码
	Vec3f ro = r.getOrigin();//射线起点
	Vec3f rd = r.getDirection();//射线方向
	float abx = _pa.x() - _pb.x();
	float aby = _pa.y() - _pb.y();
	float abz = _pa.z() - _pb.z();
	float acx = _pa.x() - _pc.x();
	float acy = _pa.y() - _pc.y();
	float acz = _pa.z() - _pc.z();
	float aox = _pa.x() - ro.x();
	float aoy = _pa.y() - ro.y();
	float aoz = _pa.z() - ro.z();

	float A = det3(abx, acx, rd.x(), aby, acy, rd.y(), abz, acz, rd.z());//|A|
	float beta = det3(aox, acx, rd.x(), aoy, acy, rd.y(), aoz, acz, rd.z()) / A;
	float gamma = det3(abx, aox, rd.x(), aby, aoy, rd.y(), abz, aoz, rd.z()) / A;
	
	if (beta > 0 && gamma > 0 && (beta + gamma) < 1) {
		
		float t = det3(abx, acx, aox, aby, acy, aoy, abz, acz, aoz) / A;
		//DEBUG 11/25 问题在h.getT()
		if (t > tmin&& t < h.getT()) {

			h.set(t, _material, _v_norm, r);
			return true;
			
		}
	}
	return false;
}
// 三角形还有另一种写法，理论上是可行的，但是有bug，放在代码末尾了。
//<<<<<<<<<<TRIANGLE<<<<<<<<<<

//>>>>>>>>>>TRANSFORM>>>>>>>>>>
Transform::Transform(Matrix &mat, Object3D *o) :_mat(mat),_obj(o){}

bool Transform::intersect(const Ray &r, Hit &h, float tmin) {
	//The intersect routine will first transform the ray, 
	//then delegate to the intersect routine of the contained object.

	//根据变换矩阵的逆矩阵来分别变换光线的Rd和Ro
	Vec3f Ro = r.getOrigin();
	Vec3f Rd = r.getDirection();
	//DEBUG
	Matrix inv_mat = _mat;
	if (!inv_mat.Inverse()) return false;
	inv_mat.Transform(Ro);
	inv_mat.TransformDirection(Rd);

	Ray inv_ray(Ro, Rd);

	//注意：这里的Hit类 h 不能是新建的h，而应该是直接引用的h。
	//为什么呢？之前新建Hit hit也能跑通啊？
	//因为之前每一个obj都只由一个primitive构成，
	//每个primitive之间可以解耦，所以primitive内的h不需一定是继承的
	//但是碰到一个obj由多个primitive构成的情况时，在一类的primitive
	//里需要进行多次比较，就需要用到历史的hit数据，这时hit不是解耦的
	//历史的hit是有意义的（比如兔子mesh前面的三角形挡住了后面的三角形）
	//因此在transform内的hit是不能新建的。
	//
	//纪念11/24（现在已经11/25 0:41了）debug的一天，还是有所收获的。

	//由多个primitives构成（如bunny）
	if (_obj->intersect(inv_ray, h, tmin)) {
		// 记得要把法线移回世界坐标系
		// 这个法线是变换后的ray照射得到的，是相对物体的坐标系
		Vec3f inv_v_norm = h.getNormal();
		Matrix _inv_mat_t = inv_mat;
		_inv_mat_t.Transpose();
		// 逆变换
		_inv_mat_t.TransformDirection(inv_v_norm);
		inv_v_norm.Normalize();

		//// 深度信息（t）也要变换回去
		//// 这一步有点没搞懂,估计是变换本身也包含scale？
		//float ori_t = h_curr.getT();
		//float t = ori_t / scale;
		h = Hit(h.getT(), h.getMaterial(), inv_v_norm);
		return true;
	}
	return false;
}

//<<<<<<<<<<TRANSFORM<<<<<<<<<<









/*
三角形intersection的另一种写法
Hit h_curr;
	Vec3f p_intersect, edge_a, edge_b, edge_c,v_p, v_curr;
	edge_a = _pa - _pb;
	edge_b = _pb - _pc;
	edge_c = _pc - _pa;

	_v_norm.show();
	if (_plane.intersect(r, h_curr, tmin)) {
		cout << "loop check" << endl;
		if (h_curr.getT() >= tmin) {
			// Check if the intersection is inside the triangle
			Vec3f p_intersect = h_curr.getIntersectionPoint();
			cout << "my t:" << h_curr.getT() << endl;
			// Check if it's inside edge a:
			v_p = p_intersect - _pa;
			Vec3f::Cross3(v_curr, v_p, edge_a);
			if (v_curr.Dot3(_v_norm) < 0)
				return false;
			// Check if it's inside edge b:
			v_p = p_intersect - _pb;
			Vec3f::Cross3(v_curr, v_p, edge_b);
			if (v_curr.Dot3(_v_norm) < 0)
				return false;
			// Check if it's inside edge c:
			v_p = p_intersect - _pc;
			Vec3f::Cross3(v_curr, v_p, edge_c);
			if (v_curr.Dot3(_v_norm) < 0)
				return false;
			h = h_curr;
			return true;
		}
		return false;
	}
	return false;
*/













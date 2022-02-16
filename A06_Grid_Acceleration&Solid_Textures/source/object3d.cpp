#include "object3d.h"
#include "raytracing_stats.h"
#include <math.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

const float PI = 3.14159265358979323846;
float MAXIMUM = numeric_limits<float>::max();
extern bool visualize_grid;

#define min3(a, b, c) (((a)<(b))?(((a)<(c))?(a):(c)):(((b)<(c))?(b):(c)))
#define max3(a, b, c) (((a)>(b))?(((a)>(c))?(a):(c)):(((b)>(c))?(b):(c)))

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

void Object3D::insertIntoGrid(Grid *g, Matrix *m) {
	if (_bbox == nullptr)
		return;
	assert(_bbox != nullptr);
	BoundingBox *bb_new = _bbox;
	//cout << bb_new->getMin() << " " << bb_new->getMax() << endl;
	//����б任�����ȴ���任m
	//����bbox�任�Ĺ�����һ����
	if (m) {
		Vec3f obj_min = _bbox->getMin();
		Vec3f obj_max = _bbox->getMax();
		float x1 = obj_min.x(), y1 = obj_min.y(), z1 = obj_min.z();
		float x2 = obj_max.x(), y2 = obj_max.y(), z2 = obj_max.z();
		Vec3f v1 = Vec3f(x1, y1, z1);
		Vec3f v2 = Vec3f(x1, y1, z2);
		Vec3f v3 = Vec3f(x1, y2, z1);
		Vec3f v4 = Vec3f(x1, y2, z2);
		Vec3f v5 = Vec3f(x2, y1, z1);
		Vec3f v6 = Vec3f(x2, y1, z2);
		Vec3f v7 = Vec3f(x2, y2, z1);
		Vec3f v8 = Vec3f(x2, y2, z2);
		m->Transform(v1);
		m->Transform(v2);
		m->Transform(v3);
		m->Transform(v4);
		m->Transform(v5);
		m->Transform(v6);
		m->Transform(v7);
		m->Transform(v8);
		bb_new = new BoundingBox(Vec3f(INFINITY, INFINITY, INFINITY), Vec3f(-INFINITY, -INFINITY, -INFINITY));
		bb_new->Extend(v1);
		bb_new->Extend(v2);
		bb_new->Extend(v3);
		bb_new->Extend(v4);
		bb_new->Extend(v5);
		bb_new->Extend(v6);
		bb_new->Extend(v7);
		bb_new->Extend(v8);
	}
	//cout << bb_new->getMin() << " " << bb_new->getMax() << endl;

	//�ⲿ��Ҳһ��
	BoundingBox *grid_bb = g->getBoundingBox();
	Vec3f grid_min = grid_bb->getMin();
	Vec3f grid_max = grid_bb->getMax();
	int nx = g->_nx;
	int ny = g->_ny;
	int nz = g->_nz;
	float cellx = (grid_max - grid_min).x() / float(nx);
	float celly = (grid_max - grid_min).y() / float(ny);
	float cellz = (grid_max - grid_min).z() / float(nz);
	Vec3f obj_min = bb_new->getMin();
	Vec3f obj_max = bb_new->getMax();
	int start_i = int((obj_min - grid_min).x() / cellx);
	int start_j = int((obj_min - grid_min).y() / celly);
	int start_k = int((obj_min - grid_min).z() / cellz);
	int end_i = int((obj_max - grid_min).x() / cellx);
	int end_j = int((obj_max - grid_min).y() / celly);
	int end_k = int((obj_max - grid_min).z() / cellz);
	if (start_i > nx || start_j > ny || start_k > nz || end_i < 0 || end_j < 0 || end_k < 0)
		return;
	start_i = max(start_i, 0);
	start_j = max(start_j, 0);
	start_k = max(start_k, 0);
	end_i = min(end_i, nx - 1);
	end_j = min(end_j, ny - 1);
	end_k = min(end_k, nz - 1);
	//note this
	if (start_i == nx) start_i--;
	if (start_j == ny) start_j--;
	if (start_k == nz) start_k--;

	for (int k = start_k; k <= end_k; ++k) {
		for (int j = start_j; j <= end_j; ++j) {
			for (int i = start_i; i <= end_i; ++i) {
				int index = nx * ny * k + nx * j + i;
				//You'll need to store the accumulated transformation matrix with each 
				//primitive. You can do this by creating a new Transformation that wraps  
				//around the primitive Object3D and store that in the Grid.
				//������Grid����ı任Ӧ����OK��
				//�������屾��ı任��û�������Ҫ��֮ǰ���Ӻõı任Wrap��obj�ϣ��Ž�opaque
				//����������intersectʱ�������ȡ����Щobj����intersectcheck����ʱ����Ҫ�����trans mat��
				g->opaque[index].push_back(new Transform(*m, this));
				//��������ä��һ����Sphere����������Ǳ��ص�grid match
			}
		}
	}
}


//>>>>>>>>>>GRID>>>>>>>>>>
//this is key function of Grid
bool Grid::intersect(const Ray &r, Hit &hit, float tmin) {
	bool flag_grid = false;
	is_intersected.clear();

	MarchingInfo mi;
	initializeRayMarch(mi, r, tmin);

	//�󽻼��
	if (mi.t_curr < hit.getT()) {
		while (mi.i >= 0 && mi.j >= 0 && mi.k >= 0 && mi.i < _nx && mi.j < _ny && mi.k < _nz) {
			int i = mi.i;
			int j = mi.j;
			int k = mi.k;
			int index = _nx * _ny * k + _nx * j + i;
			//�����ӻ�����
			if (!visualize_grid && !opaque[index].empty()) {
				for (Object3D* obj : opaque[index]) {
					//�ظ���⣬���֮ǰ��ǹ���ֱ������
					if (is_intersected.find(obj) != is_intersected.end())
						continue;
					//�����ǰobjû���ཻ�����ǣ��´������ж�
					if (!obj->intersect(r, hit, tmin))
						is_intersected.insert(obj);
				}
				if (hit.getT() < min3(mi.t_next_x, mi.t_next_y, mi.t_next_z) + 0.001) {
					//Ӧ���ǵ�ǰ�������¸�cell֮ǰ��������Ч�����ˣ����Է���
					//���flag_grid��ʲô�ã�
					flag_grid = true;
					break;
				}
			}
			//���ӻ�����
			//�Ͳ��ü��㸴�ӵ�intersection�ˣ�ֱ�Ӹ�������ɫ����
			if (visualize_grid && !opaque[index].empty()) {
				PhongMaterial *m;
				switch (opaque[index].size()) {
				case 1: m = new PhongMaterial(Vec3f(1, 1, 1)); break;
				case 2: m = new PhongMaterial(Vec3f(1, 0, 1)); break;
				case 3: m = new PhongMaterial(Vec3f(0, 1, 1)); break;
				case 4: m = new PhongMaterial(Vec3f(1, 1, 0)); break;
				case 5: m = new PhongMaterial(Vec3f(0.3, 0, 0.7)); break;
				case 6: m = new PhongMaterial(Vec3f(0.7, 0, 0.3)); break;
				case 7: m = new PhongMaterial(Vec3f(0, 0.3, 0.7)); break;
				case 8: m = new PhongMaterial(Vec3f(0, 0.7, 0.3)); break;
				case 9: m = new PhongMaterial(Vec3f(0, 0.3, 0.7)); break;
				case 10: m = new PhongMaterial(Vec3f(0, 0.7, 0.3)); break;
				case 11: m = new PhongMaterial(Vec3f(0, 1, 0)); break;
				case 12: m = new PhongMaterial(Vec3f(0, 0, 1)); break;
				default: m = new PhongMaterial(Vec3f(1, 0, 0)); break;
				}
				hit.set(mi.t_curr, m, mi.normal, r);
				return true;
			}
			//��������ŵ�ǰ�Ĺ�·�Զ���������t_curr
			mi.nextcell();
		}
	}

	//���Ӷ���Plane����infinite primitives�Ĵ���
	if (!visualize_grid) {
		//������ʾ��ǰ�����Ƿ���Plane�ཻ
		bool flag_infinite = false;
		Hit hit_infinite(INFINITY);
		for (Object3D* obj : infinitePrimitives) {
			if (obj->intersect(r, hit_infinite, tmin)) {
				flag_infinite = true;
			}
		}
		//�����ǰ����δ��Plane�ཻ����ֱ����������Ľ����Ҳ����flag_grid
		if (!flag_infinite) {
			return flag_grid;
		}
		//�������Ҳû�м�⵽�ཻ����ʹ�������ƽ����ཻhit
		if (!flag_grid) {
			hit = hit_infinite;
			return flag_infinite; //true
		}
		//��������м�⵽��Чhit��ƽ��Ҳ�м�⵽��Чhit����Ա����ߴ�С��ȡ������
		if (hit_infinite.getT() < hit.getT()) {
			hit = hit_infinite;
			return true;
		}
		else {
			return true;
		}
	}
	return false;
}

bool Grid::intersectShadowRay(const Ray &r, Hit &hit, float tmin) {
	// is_intersected ��һ��std::set
	//���ڲ��������е�Ԫ��ʼ�հ������ڲ��Ƚ϶�������ΪCompare ��ָʾ���ض��ϸ��������׼����������������ͨ����
	//unordered_set����ͨ�����ǵ�key���ʵ���Ԫ�أ�����������������ǵ�˳����Ӽ�����ֱ�ӵ�����
	//setͨ��ʵ��Ϊ������������
	//����о�����Ϊһ����ϣ����ʹ�ã�����Ҳ�����滻��unordered map�ɣ�
	is_intersected.clear();
	MarchingInfo mi;
	initializeRayMarch(mi, r, tmin);
	if (mi.t_curr < hit.getT()) {
		while (mi.i >= 0 && mi.j >= 0 && mi.k >= 0 && mi.i < _nx && mi.j < _ny && mi.k < _nz) {
			int i = mi.i;
			int j = mi.j;
			int k = mi.k;
			int index = _nx * _ny * k + _nx * j + i;
			if (!opaque[index].empty()) { 
				for (Object3D *obj : opaque[index]) {
					if (is_intersected.find(obj) != is_intersected.end())
						//�����is_intersected.end()Ӧ���ǿ�ָ�룬Ҳ����NULL
						//�������if����˼�ǣ����֮ǰ�Ѿ����������obj�����������
						continue;
					if (!obj->intersect(r, hit, tmin))
						//��obj����Ĺ���û�б��ڵ�������
						is_intersected.insert(obj);
					else
						//��obj����Ĺ��߱��ڵ��ˣ�Ϊ��Ӱ
						return true;
				}
			}
			//����
			mi.nextcell();
		}
	}
	//���Ͻ���ٵ�����Plane��һ��intersect check
	for (Object3D *obj : infinitePrimitives) {
		if (obj->intersect(r, hit, tmin))
			return true;
	}
	return false;
}

void Grid::paint() {
	//���������в�͸���ĸ���
	Vec3f minP = _bbox->getMin();
	Vec3f maxP = _bbox->getMax();
	//���㵥λ����
	float cellx = (maxP - minP).x() / float(_nx);
	float celly = (maxP - minP).y() / float(_ny);
	float cellz = (maxP - minP).z() / float(_nz);
	for (int k = 0; k < _nz; k++) {
		for (int j = 0; j < _ny; j++) {
			for (int i = 0; i < _nx; i++) {
				//��ǰindex
				int index = k * _ny*_nx + j * _nx + i;
				if (!opaque[index].empty()) {
					//�ȼ��㵱ǰ���Ӱ˸��������
					Vec3f a = minP + Vec3f(i * cellx, j * celly, k * cellz);
					Vec3f b = minP + Vec3f((i + 1) * cellx, j * celly, k * cellz);
					Vec3f c = minP + Vec3f((i + 1) * cellx, (j + 1) * celly, k * cellz);
					Vec3f d = minP + Vec3f(i * cellx, (j + 1) * celly, k * cellz);
					Vec3f e = minP + Vec3f(i * cellx, j * celly, (k + 1) * cellz);
					Vec3f f = minP + Vec3f((i + 1) * cellx, j * celly, (k + 1) * cellz);
					Vec3f g = minP + Vec3f((i + 1) * cellx, (j + 1) * celly, (k + 1) * cellz);
					Vec3f h = minP + Vec3f(i * cellx, (j + 1) * celly, (k + 1) * cellz);
					//��ʼ��һ��Materialָ�룬�����浱ǰ���ӵ���ɫ
					PhongMaterial *m;
					//�����opaque[index].size()�ǵ�ǰ��Ԫ�����ཻobject�ĸ���
					switch (opaque[index].size()) {
					case 1: m = new PhongMaterial(Vec3f(1, 1, 1)); break;
					case 2: m = new PhongMaterial(Vec3f(1, 0, 1)); break;
					case 3: m = new PhongMaterial(Vec3f(0, 1, 1)); break;
					case 4: m = new PhongMaterial(Vec3f(1, 1, 0)); break;
					case 5: m = new PhongMaterial(Vec3f(0.3, 0, 0.7)); break;
					case 6: m = new PhongMaterial(Vec3f(0.7, 0, 0.3)); break;
					case 7: m = new PhongMaterial(Vec3f(0, 0.3, 0.7)); break;
					case 8: m = new PhongMaterial(Vec3f(0, 0.7, 0.3)); break;
					case 9: m = new PhongMaterial(Vec3f(0, 0.3, 0.7)); break;
					case 10: m = new PhongMaterial(Vec3f(0, 0.7, 0.3)); break;
					case 11: m = new PhongMaterial(Vec3f(0, 1, 0)); break;
					case 12: m = new PhongMaterial(Vec3f(0, 0, 1)); break;
					default: m = new PhongMaterial(Vec3f(1, 0, 0)); break;
					}
					//openGL�������������
					m->glSetMaterial();
					glBegin(GL_QUADS);
					glNormal3f(0, 0, -1);
					glVertex3f(a.x(), a.y(), a.z());
					glVertex3f(b.x(), b.y(), b.z());
					glVertex3f(c.x(), c.y(), c.z());
					glVertex3f(d.x(), d.y(), d.z());
					glNormal3f(0, 0, 1);
					glVertex3f(e.x(), e.y(), e.z());
					glVertex3f(f.x(), f.y(), f.z());
					glVertex3f(g.x(), g.y(), g.z());
					glVertex3f(h.x(), h.y(), h.z());
					glNormal3f(1, 0, 0);
					glVertex3f(b.x(), b.y(), b.z());
					glVertex3f(c.x(), c.y(), c.z());
					glVertex3f(g.x(), g.y(), g.z());
					glVertex3f(f.x(), f.y(), f.z());
					glNormal3f(-1, 0, 0);
					glVertex3f(a.x(), a.y(), a.z());
					glVertex3f(d.x(), d.y(), d.z());
					glVertex3f(h.x(), h.y(), h.z());
					glVertex3f(e.x(), e.y(), e.z());
					glNormal3f(0, -1, 0);
					glVertex3f(a.x(), a.y(), a.z());
					glVertex3f(b.x(), b.y(), b.z());
					glVertex3f(f.x(), f.y(), f.z());
					glVertex3f(e.x(), e.y(), e.z());
					glNormal3f(0, 1, 0);
					glVertex3f(c.x(), c.y(), c.z());
					glVertex3f(d.x(), d.y(), d.z());
					glVertex3f(h.x(), h.y(), h.z());
					glVertex3f(g.x(), g.y(), g.z());
					glEnd();
				}
			}
		}
	}
}

void Grid::initializeRayMarch(MarchingInfo &mi, const Ray &r, float tmin) {
	//�ú�����������ߺ�Grid�ĳ�ʼ����
	Vec3f ro = r.getOrigin();
	Vec3f rd = r.getDirection();
	Vec3f minP = _bbox->getMin();
	Vec3f maxP = _bbox->getMax();
	//���㵥λ����
	float cellx = (maxP - minP).x() / _nx;
	float celly = (maxP - minP).y() / _ny;
	float cellz = (maxP - minP).z() / _nz;

	//��������һЩ���ݴ���MarchingInfo

	//dt
	mi.dt_x = fabs(cellx / rd.x());//����rd.x()�Ǳ�׼�����ģ���ͬ�ڽǶȵ�cos
	mi.dt_y = fabs(celly / rd.y());
	mi.dt_z = fabs(cellz / rd.z());

	//sign
	mi.sign_x = rd.x() > 0 ? 1 : -1;
	mi.sign_y = rd.y() > 0 ? 1 : -1;
	mi.sign_z = rd.z() > 0 ? 1 : -1;

	//t_cur
	float t_near, t_far;
		//����rd.x()�ĳ�(rd-ro).x()�ǲ���Ҳ����
	float t1_x = (minP - ro).x() / rd.x();//����bbox����С��������ڹ�·�е�tֵ��0~1��
	float t2_x = (maxP - ro).x() / rd.x();//ͬ��
	//���rd����Ϊ�����������߽���
	if (rd.x() < 0)
		swap(t1_x, t2_x);
	float t1_y = (minP - ro).y() / rd.y();
	float t2_y = (maxP - ro).y() / rd.y();
	if (rd.y() < 0)
		swap(t1_y, t2_y);
	float t1_z = (minP - ro).z() / rd.z();
	float t2_z = (maxP - ro).z() / rd.z();
	if (rd.z() < 0)
		swap(t1_z, t2_z);
	//��������AABB�󽻣�������Կ��ʼ�
	t_near = max3(t1_x, t1_y, t1_z);
	t_far = min3(t2_x, t2_y, t2_z);
	if (t_near >= t_far) //���ཻ
		return;
	if (t_far <= tmin) //�����ڹ۲���
		return;
	//Inside the object
	if (t_near < tmin) {
		//��������dt��ֱ����ǰ��t���ڵ���tmin
		if (t1_x > -INFINITY) {
			while (t1_x < tmin)
				t1_x += mi.dt_x;
		}
		if (t1_y > -INFINITY) {
			while (t1_y < tmin)
				t1_y += mi.dt_y;
		}
		if (t1_z > -INFINITY) {
			while (t1_z < tmin)
				t1_z += mi.dt_z;
		}
		//���t1����tmin��ֱ����������Ϊɶ����
		t1_x = t1_x > tmin ? t1_x : INFINITY;
		t1_y = t1_y > tmin ? t1_y : INFINITY;
		t1_z = t1_z > tmin ? t1_z : INFINITY;
		//ע�⣬�����t_near�������t_near�Ѿ���һ����
		//�����if��֧�£������t_near����t1���������е���Сֵ
		t_near = min3(t1_x, t1_y, t1_z);
		mi.t_curr = t_near;
		//�ο�marchinginfo�е�3DDDA
		if (t_near == t1_x) {
			mi.normal = Vec3f(-1, 0, 0) * mi.sign_x;
			mi.t_next_x = t1_x + mi.dt_x;
			mi.t_next_y = t1_y;
			mi.t_next_z = t1_z;
		}
		if (t_near == t1_y) {
			mi.normal = Vec3f(0, -1, 0) * mi.sign_y;
			mi.t_next_x = t1_x;
			mi.t_next_y = t1_y + mi.dt_y;
			mi.t_next_z = t1_z;
		}
		if (t_near == t1_z) {
			mi.normal = Vec3f(0, 0, -1) * mi.sign_z;
			mi.t_next_x = t1_x;
			mi.t_next_y = t1_y;
			mi.t_next_z = t1_z + mi.dt_z;
		}
	}
	//Outside the object
	else {
		mi.t_curr = t_near;
		if (t_near == t1_x) mi.normal = Vec3f(-1, 0, 0) * mi.sign_x;
		if (t_near == t1_y) mi.normal = Vec3f(0, -1, 0) * mi.sign_y;
		if (t_near == t1_z) mi.normal = Vec3f(0, 0, -1) * mi.sign_z;
		//mi.t_next_x
		if (t1_x > -INFINITY) {
			while (t1_x <= t_near)
				t1_x += mi.dt_x;
			mi.t_next_x = t1_x;
		}
		if (t1_y > -INFINITY) {
			while (t1_y <= t_near)
				t1_y += mi.dt_y;
			mi.t_next_y = t1_y;
		}
		if (t1_z > -INFINITY) {
			while (t1_z <= t_near)
				t1_z += mi.dt_z;
			mi.t_next_z = t1_z;
		}
	}

	//Index i,j,k
	Vec3f p = ro + rd * t_near - minP;//��ǰ�����Grid��С��λ��
	mi.i = int(p.x() / cellx);
	if (mi.sign_x < 0 && mi.i == _nx) mi.i--;
	mi.j = int(p.y() / celly);
	if (mi.sign_y < 0 && mi.j == _ny) mi.j--;
	mi.k = int(p.z() / cellz);
	if (mi.sign_z < 0 && mi.k == _nz) mi.k--;
}


//<<<<<<<<<<GRID<<<<<<<<<<<


//>>>>>>>>>>GROUP>>>>>>>>>>
Group::Group(const int num) :_n(num) {
	_bbox = new BoundingBox(Vec3f(MAXIMUM, MAXIMUM, MAXIMUM), Vec3f(-MAXIMUM, -MAXIMUM, -MAXIMUM));
};

void Group::addObject(int index, Object3D *obj) {
	assert(index < _n);
	objArray.push_back(obj);
	if (obj->getBoundingBox())
		_bbox->Extend(obj->getBoundingBox());
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

bool Group::intersectShadowRay(const Ray &r, Hit &h, float tmin) {
	for (int i = 0; i < objArray.size(); i++) {
		if (objArray[i]->intersect(r, h, tmin)) {
			return true;
		}
	}
	return false;
}

void Group::insertIntoGrid(Grid *g, Matrix *m) {
	for (int i = 0; i < _n; i++) {
		objArray[i]->insertIntoGrid(g, m);
	}
}

void Group::paint() {
	for (auto obj : objArray)
		obj->paint();
}
//<<<<<<<<<<GROUP<<<<<<<<<<<



//>>>>>>>>>>SPHERE>>>>>>>>>>
Sphere::Sphere(Vec3f &center, float &radius, Material* material) :_center(center), _radius(radius) {
	// ����ĳ�ʼ���б��ܳ�ʼ������ĳ�Ա
	//https://blog.csdn.net/yanlei208/article/details/101089966
	_material = material; //�������Ǹ�ֵ��ʼ��
	_bbox = new BoundingBox(Vec3f(_center.x() - _radius, _center.y() - _radius, _center.z() - _radius), Vec3f(_center.x() + _radius, _center.y() + _radius, _center.z() + _radius));

}

bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
	RayTracingStats::IncrementNumIntersections();
	Vec3f Ro = r.getOrigin() - _center;
	Vec3f Rd = r.getDirection();
	float a = Rd.Dot3(Rd);
	float b = 2 * Ro.Dot3(Rd);
	float c = Ro.Dot3(Ro) - _radius * _radius;
	float delta = b * b - 4 * a * c;
	float t = INFINITY; //t�ĳ�ʼֵӦ��������Զ��
	if (delta >= 0) {
		float d = sqrt(delta);
		float t1 = (-b - d) / (2 * a);
		float t2 = (-b + d) / (2 * a);
		// Add normal vector calculation:
		// Hit point - center
		if (t1 >= tmin) {
			t = t1;
		}
		else if (t2 >= tmin) {
			t = t2;
		}
		if (t < h.getT()) {//closer��ǰ���㣬����
			//Vec3f v_norm = Vec3f(r.pointAtParameter(t), _center);//HACK:���ӷ������ļ���
			Vec3f v_norm = Ro + t * Rd;
			v_norm.Normalize();
			//h = Hit(t, _material, v_norm);//���������
			h.set(t, _material, v_norm, r);
			return true;
		}
	}
	return false;
}

extern int theta_steps;
extern int phi_steps;
extern bool gouraud;

void Sphere::paint() {
	_material->glSetMaterial();
	float dt = 2 * PI / theta_steps; // 0-360 / step
	float dp = PI / phi_steps; // -90~+90 / step
	vector<Vec3f> position;
	vector<Vec3f> normal;
	// ��ʼ�����е��λ��
	for (int p = 0; p <= phi_steps; p++) {
		for (int t = 0; t < theta_steps; t++) {
			float theta = t * dt;
			float phi = p * dp;
			Vec3f pos = Vec3f(sin(phi)*cos(theta),cos(phi), sin(phi)*sin(theta));
			//pos.show();
			position.push_back(_center + pos * _radius);
			normal.push_back(pos);
		}
	}

	glBegin(GL_QUADS);
	for (int phi = 0; phi < phi_steps; phi++) {
		for (int theta = 0; theta < theta_steps; theta++) {
			int index[4] = { phi*theta_steps + theta,
							(phi + 1)*theta_steps + theta,
							(phi + 1)*theta_steps + (theta + 1) % theta_steps,
							phi*theta_steps + (theta + 1) % theta_steps };
			Vec3f n;
			if (!gouraud) {
				// ��һ�֣����meshƽ��ķ���
				Vec3f::Cross3(n, position[index[0]] - position[index[1]], position[index[2]] - position[index[1]]);
				glNormal3f(n.x(), n.y(), n.z());
			}
			for (int i = 0; i < 4; i++) {
				if (gouraud) {
					// �ڶ��֣�����ֱ�Ӿ�����İ뾶��������
					glNormal3f(normal[index[i]].x(), normal[index[i]].y(), normal[index[i]].z());
				}
				glVertex3f(position[index[i]].x(), position[index[i]].y(), position[index[i]].z());
			}

		}
	}
	glEnd();
}

void Sphere::insertIntoGrid(Grid *g, Matrix *m) {
	//����Ƕ�룿
	if (m) {
		//����б任���ȴ���任������任��
		Object3D::insertIntoGrid(g, m);
		return;
	}
	BoundingBox *grid_bb = g->getBoundingBox();
	//��ȡȫ���������С���ĶԽǵ�BBOX
	Vec3f grid_min = grid_bb->getMin();
	Vec3f grid_max = grid_bb->getMax();
	//��ȡ����������ֱ������
	int nx = g->_nx;
	int ny = g->_ny;
	int nz = g->_nz;
	//ÿһ������ĵ�Ԫ�񳤶�=�ܳ�/������
	float cellx = (grid_max - grid_min).x() / float(nx);
	float celly = (grid_max - grid_min).y() / float(ny);
	float cellz = (grid_max - grid_min).z() / float(nz);
	//ÿ�����ӵĶԽ��߳��ȣ������ʲô�ã���
	float diagonal = sqrt(cellx * cellx + celly * celly + cellz * cellz);

	//�����ཻ�ж�
	for (int k = 0; k < nz; k++) {
		for (int j = 0; j < ny; j++) {
			for (int i = 0; i < nx; i++) {
				//ÿһ��cell���������꣺
				float cx = (i+0.5) * cellx;
				float cy = (j+0.5) * celly;
				float cz = (k+0.5) * cellz;
				//�ж��Ƿ��ཻ(����ҪС�ڰ뾶+��Ԫ��Խ��߳�)
				if (((Vec3f(cx, cy, cz) + grid_min) - _center).Length() < _radius + diagonal / 2) {
				//if ((Vec3f(cx, cy, cz) - (_center - grid_min)).Length() < _radius + diagonal/2) {
					//��Ӧarray�е�index��
					int index = i + ny * j + nx * ny * k;
					//ά��һ����opaque��vector�����vector�е�ĳһ��λ�ÿ������ڶ��primitives�����Ի�Ҫ��һ��vector
					//Ҳ������ˣ������ҵ���Ӧ��opaque��λ�󣬽���ǰobject pushback���ڶ���vector��
					g->opaque[index].push_back(this);
				}
			}
		}
	}
}
//<<<<<<<<<<SPHERE<<<<<<<<<<



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
	RayTracingStats::IncrementNumIntersections();
	// ע�⣺Vec3f���ʱ�����Ӧ�Ǹ�������������ܱ�ǿ��ת����0
	Vec3f Ro = r.getOrigin();
	Vec3f Rd = r.getDirection();
	float denom = _v_norm.Dot3(Rd);
	if (fabs(denom) < 0.00001) return false;//��ĸ����Ϊ0
	float t = (_d - _v_norm.Dot3(Ro)) / denom;
	if (t > tmin && t < h.getT()) {
		h.set(t, _material, _v_norm,r);
		return true;
	}
	return false;
}

void Plane::paint() {
	Vec3f n = _v_norm;
	Vec3f v;
	Vec3f u = Vec3f(1.f, 0.f, 0.f); //�ȼ���vΪx����λ����
	n.Normalize();

	// �жϷ�����n�Ƿ��vƽ�У�����ǣ���Ҫ��һ��v��
	if (fabs(n.Dot3(u) - 1) < 0.01)
		u = Vec3f(0.f, 1.f, 0.f);

	Vec3f::Cross3(v, n, u);
	Vec3f::Cross3(u, v, n);

	// �������ƽ��ĳ�����ε��ĸ���
	Vec3f center = n * _d; //���ĵ������ŷ�������ƫ��d�ľ�������Ӧ�ĵ�
	const int INF = 100;
	//DEBUG
	//TODO: �������û����vector����֪��Ϊɶ
	//vector<Vec3f> Rect;
	//Rect[0] = center - u * INF - v * INF;
	//Rect[1] = center - u * INF + v * INF;
	//Rect[2] = center + u * INF - v * INF;
	//Rect[3] = center + u * INF + v * INF;
	//Rect[0].show();
	//Rect[1].show();
	//Rect[2].show();
	//Rect[3].show();
	//cout << "debug" << endl;
	_material->glSetMaterial();
	Vec3f Rect[4] = { center + INF * u, center + INF * v, center - INF * u, center - INF * v };

	// ����OpenGL����
	glBegin(GL_QUADS);
	glNormal3f(_v_norm.x(), _v_norm.y(), _v_norm.z());
	glVertex3f(Rect[0].x(), Rect[0].y(), Rect[0].z());
	glVertex3f(Rect[1].x(), Rect[1].y(), Rect[1].z());
	glVertex3f(Rect[2].x(), Rect[2].y(), Rect[2].z());
	glVertex3f(Rect[3].x(), Rect[3].y(), Rect[3].z());
	glEnd();
}

void Plane::insertIntoGrid(Grid *g, Matrix *m) {
	g->infinitePrimitives.push_back(this);
}
//<<<<<<<<<<PLANE<<<<<<<<<<


//>>>>>>>>>>TRIANGLE>>>>>>>>>>
Triangle::Triangle(Vec3f &a, Vec3f &b, Vec3f &c, Material *m) {
	_pa = a;
	_pb = b;
	_pc = c;
	_material = m;
	Vec3f::Cross3(_v_norm, _pb-_pa, _pc-_pa); //ab������ac����
	_v_norm.Normalize();

	_bbox = new BoundingBox(Vec3f(min(_pa.x(), _pb.x()), min(_pa.y(), _pb.y()), min(_pa.z(), _pb.z())),
							Vec3f(max(_pa.x(), _pb.x()), max(_pa.y(), _pb.y()), max(_pa.z(), _pb.z())));
	_bbox->Extend(_pc);
}

bool Triangle::intersect(const Ray &r, Hit &h, float tmin) {
	RayTracingStats::IncrementNumIntersections();
	//����͵���ˣ��ο��ı��˵Ĵ���
	Vec3f ro = r.getOrigin();//�������
	Vec3f rd = r.getDirection();//���߷���
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
		//DEBUG 11/25 ������h.getT()
		if (t > tmin&& t < h.getT()) {

			h.set(t, _material, _v_norm, r);
			return true;
			
		}
	}
	return false;
}
// �����λ�����һ��д�����������ǿ��еģ�������bug�����ڴ���ĩβ�ˡ�

BoundingBox *Triangle::getTriangleBoundingBox(Matrix *m) {
	if (!m)
		return _bbox;//������������û�б任����ֱ�ӷ��س�ʼ���������bbox
	Vec3f a = _pa, b = _pb, c = _pc;
	m->Transform(a);
	m->Transform(b);
	m->Transform(c);
	BoundingBox *bbox_t = new BoundingBox(Vec3f(min(a.x(), b.x()), min(a.y(), b.y()), min(a.z(), b.z())),
										  Vec3f(max(a.x(), b.x()), max(a.y(), b.y()), max(a.z(), b.z())));
	bbox_t->Extend(c);
	return bbox_t;
}

void Triangle::paint() {
	_material->glSetMaterial();
	glBegin(GL_TRIANGLES);
	glNormal3f(_v_norm.x(), _v_norm.y(), _v_norm.z());
	glVertex3f(_pa.x(), _pa.y(), _pa.z());
	glVertex3f(_pb.x(), _pb.y(), _pb.z());
	glVertex3f(_pc.x(), _pc.y(), _pc.z());
	glEnd();
}

void Triangle::insertIntoGrid(Grid *g, Matrix *m) {
	//�任���������bbox
	BoundingBox *bb_new = getTriangleBoundingBox(m);
	//����grid����Ϣ
	BoundingBox *grid_bb = g->getBoundingBox();
	Vec3f grid_min = grid_bb->getMin();
	Vec3f grid_max = grid_bb->getMax();
	int nx = g->_nx;
	int ny = g->_ny;
	int nz = g->_nz;
	float cellx = (grid_max - grid_min).x() / float(nx);
	float celly = (grid_max - grid_min).y() / float(ny);
	float cellz = (grid_max - grid_min).z() / float(nz);
	Vec3f obj_min = bb_new->getMin();
	Vec3f obj_max = bb_new->getMax();
	int start_i = int((obj_min - grid_min).x() / cellx);
	int start_j = int((obj_min - grid_min).y() / celly);
	int start_k = int((obj_min - grid_min).z() / cellz);
	int end_i = int((obj_max - grid_min).x() / cellx);
	int end_j = int((obj_max - grid_min).y() / celly);
	int end_k = int((obj_max - grid_min).z() / cellz);
	//�����ж�
	if (start_i > nx || start_j > ny || start_k > nz || end_i < 0 || end_j < 0 || end_k < 0)
		return;
	start_i = max(start_i, 0);
	start_j = max(start_j, 0);
	start_k = max(start_k, 0);
	end_i = min(end_i, nx - 1);
	end_j = min(end_j, ny - 1);
	end_k = min(end_k, nz - 1);
	//note this
	if (start_i == nx) start_i--;
	if (start_j == ny) start_j--;
	if (start_k == nz) start_k--;
	//�����ƺ���ȡֱ��ȡ������bbox��Ϊgrid�е�������Ϊ�±���ؼ���forѭ����û��if�ж�
	for (int k = start_k; k <= end_k; ++k) {
		for (int j = start_j; j <= end_j; ++j) {
			for (int i = start_i; i <= end_i; ++i) {
				int index = nx * ny * k + nx * j + i;
				if (m)
					g->opaque[index].push_back(new Transform(*m, this));
				else
					g->opaque[index].push_back(this);
			}
		}
	}
}
//<<<<<<<<<<TRIANGLE<<<<<<<<<<

//>>>>>>>>>>TRANSFORM>>>>>>>>>>
Transform::Transform(Matrix &mat, Object3D *o) :_mat(mat),_obj(o){
	_bbox = o->getBoundingBox();
}

BoundingBox *Transform::getBoundingBox() {
	assert(_bbox != nullptr);
	if (_obj->is_triangle())
		return _obj->getTriangleBoundingBox(&_mat);
	//̫�������棺
	//���ȣ����ԭbbox��Ӧ��8��������ǵ�
	//��Σ�����8���ǵ�ֱ����任
	//����ټ�����8���½ǵ�����Ӧ��bbox
	Vec3f obj_min = _bbox->getMin();
	Vec3f obj_max = _bbox->getMax();
	float x1 = obj_min.x(), y1 = obj_min.y(), z1 = obj_min.z();
	float x2 = obj_max.x(), y2 = obj_max.y(), z2 = obj_max.z();
	Vec3f v1 = Vec3f(x1, y1, z1);
	Vec3f v2 = Vec3f(x1, y1, z2);
	Vec3f v3 = Vec3f(x1, y2, z1);
	Vec3f v4 = Vec3f(x1, y2, z2);
	Vec3f v5 = Vec3f(x2, y1, z1);
	Vec3f v6 = Vec3f(x2, y1, z2);
	Vec3f v7 = Vec3f(x2, y2, z1);
	Vec3f v8 = Vec3f(x2, y2, z2);
	_mat.Transform(v1);
	_mat.Transform(v2);
	_mat.Transform(v3);
	_mat.Transform(v4);
	_mat.Transform(v5);
	_mat.Transform(v6);
	_mat.Transform(v7);
	_mat.Transform(v8);

	//ע�⣺������СֵΪ���޴����ֵΪ����С��ʵ������һ����ʼ����
	BoundingBox *bb_new = new BoundingBox(Vec3f(MAXIMUM, MAXIMUM, MAXIMUM), Vec3f(-MAXIMUM, -MAXIMUM, -MAXIMUM));
	bb_new->Extend(v1);
	bb_new->Extend(v2);
	bb_new->Extend(v3);
	bb_new->Extend(v4);
	bb_new->Extend(v5);
	bb_new->Extend(v6);
	bb_new->Extend(v7);
	bb_new->Extend(v8);
	return bb_new;

}

bool Transform::intersect(const Ray &r, Hit &h, float tmin) {
	//The intersect routine will first transform the ray, 
	//then delegate to the intersect routine of the contained object.

	//���ݱ任�������������ֱ�任���ߵ�Rd��Ro
	Vec3f Ro = r.getOrigin();
	Vec3f Rd = r.getDirection();
	//DEBUG
	Matrix inv_mat = _mat;
	if (inv_mat.Inverse()) {
		inv_mat.Transform(Ro);
		inv_mat.TransformDirection(Rd);

		Ray inv_ray(Ro, Rd);

		//ע�⣺�����Hit�� h �������½���h����Ӧ����ֱ�����õ�h��
		//Ϊʲô�أ�֮ǰ�½�Hit hitҲ����ͨ����
		//��Ϊ֮ǰÿһ��obj��ֻ��һ��primitive���ɣ�
		//ÿ��primitive֮����Խ������primitive�ڵ�h����һ���Ǽ̳е�
		//��������һ��obj�ɶ��primitive���ɵ����ʱ����һ���primitive
		//����Ҫ���ж�αȽϣ�����Ҫ�õ���ʷ��hit���ݣ���ʱhit���ǽ����
		//��ʷ��hit��������ģ���������meshǰ��������ε�ס�˺���������Σ�
		//�����transform�ڵ�hit�ǲ����½��ġ�
		//
		//����11/24�������Ѿ�11/25 0:41�ˣ�debug��һ�죬���������ջ�ġ�

		//�ɶ��primitives���ɣ���bunny��
		if (_obj->intersect(inv_ray, h, tmin)) {
			// �ǵ�Ҫ�ѷ����ƻ���������ϵ
			// ��������Ǳ任���ray����õ��ģ���������������ϵ
			Vec3f inv_v_norm = h.getNormal();
			Matrix _inv_mat_t = inv_mat;
			_inv_mat_t.Transpose();
			// ��任
			_inv_mat_t.TransformDirection(inv_v_norm);
			inv_v_norm.Normalize();

			//// �����Ϣ��t��ҲҪ�任��ȥ
			//// ��һ���е�û�㶮,�����Ǳ任����Ҳ����scale��
			//float ori_t = h_curr.getT();
			//float t = ori_t / scale;
			h.set(h.getT(), h.getMaterial(), inv_v_norm, r);
			return true;
		}
	}
	return false;
}

void Transform::insertIntoGrid(Grid *g, Matrix *m) {
	//����ж���任��
	if (m) {
		Matrix t_post = (*m) * _mat;
		_obj->insertIntoGrid(g, &t_post);
	}
	else {
		//����Ĭ��ʹ��Transformʵ�����õı任
		//ʵ�ʱ任���������Object::insertIntoGrid()��
		_obj->insertIntoGrid(g, &_mat);
	}
}

void Transform::paint() {
	glPushMatrix();
	GLfloat *glMatrix = _mat.glGet();
	glMultMatrixf(glMatrix);
	delete[] glMatrix;
	_obj->paint();
	glPopMatrix();
}

//<<<<<<<<<<TRANSFORM<<<<<<<<<<



/*
������intersection����һ��д��
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













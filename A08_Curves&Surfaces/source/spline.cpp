#include "spline.h"
#include <GL/freeglut.h>

Matrix Spline::Bezier_Matrix = Matrix(new float[16]{ -1,3,-3,1,3,-6,3,0,-3,3,0,0,1,0,0,0 });
Matrix Spline::BSpline_Matrix = Matrix(new float[16]{ -1,3,-3,1,3,-6,0,4,-3,3,3,1,1,0,0,0 })*(1.f / 6);

const double PI = 3.1415926;

void Curve::Paint(ArgParser *args) {
	drawLines();
	drawCurves(args);
	drawPoints();
}

void Curve::drawLines() {
	glLineWidth(3);
	glColor3f(0, 0, 1);
	glBegin(GL_LINE_STRIP);
	for (Vec3f v : vertices) {
		glVertex3f(v.x(), v.y(), v.z());
	}
	glEnd();
}

void Curve::drawPoints() {
	glPointSize(5);
	glColor3f(1, 0, 0);
	glBegin(GL_POINTS);
	for (Vec3f v : vertices) {
		glVertex3f(v.x(), v.y(), v.z());
	}
	glEnd;
}

void BezierCurve::drawCurves(ArgParser *args) {
	glLineWidth(3);
	glColor3f(0, 1, 0);
	glBegin(GL_LINE_STRIP);
	int tessellation = args->curve_tessellation;
	float d = 1.f / tessellation;
	for (int i = 0; i < num_vertices - 3; i += 3) {
		Matrix P;
		for (int j = 0; j < 4; j++) {
			P.Set(j, 0, vertices[i + j].x());
			P.Set(j, 1, vertices[i + j].y());
			P.Set(j, 2, vertices[i + j].z());
		}
		for (int j = 0; j <= tessellation; j++) {
			//步进 t，单位长度为d
			float t = d * j;
			float t2 = t * t;
			float t3 = t2 * t;
			Vec4f T(t3, t2, t, 1);
			Bezier_Matrix.Transform(T);
			P.Transform(T);// Q(t) = G * B * T,这里G就是P
			//现在T已经是最后的计算结果Q(t)了。
			glVertex3f(T.x(), T.y(), T.z());
		}
	}
	glEnd();
}

void BezierCurve::OutputBezier(FILE *file) {
	fprintf(file, "\n");
	fprintf(file, "bezier\nnum_vertices %d\n", num_vertices);
	for (Vec3f &v : vertices) {
		fprintf(file, "%f %f %f\n", v.x(), v.y(), v.z());
	}
}

void BezierCurve::OutputBSpline(FILE *file) {
	fprintf(file, "\n");
	fprintf(file, "bspline\nnum_vertices %d\n", num_vertices);
	Matrix P;
	for (int i = 0; i < num_vertices; ++i) {
		P.Set(i, 0, vertices[i].x());
		P.Set(i, 1, vertices[i].y());
		P.Set(i, 2, vertices[i].z());
	}
	Matrix inv_BSpline_Matrix = BSpline_Matrix;
	inv_BSpline_Matrix.Inverse();
	Matrix res = P * Bezier_Matrix * inv_BSpline_Matrix;//求Gbspline
	for (int i = 0; i < num_vertices; ++i) {
		fprintf(file, "%f %f %f\n", res.Get(i, 0), res.Get(i, 1), res.Get(i, 2));
	}
}

void BezierCurve::moveControlPoint(int selectedPoint, float x, float y) {
	vertices[selectedPoint].Set(x, y, 0);
}

void BezierCurve::addControlPoint(int selectedPoint, float x, float y) {}

void BezierCurve::deleteControlPoint(int selectedPoint) {}

TriangleMesh *BezierCurve::OutputTriangles(ArgParser *args) {
	int curve_tessellation = args->curve_tessellation;
	int revolution_tessellation = args->revolution_tessellation;
	float curve_d = 1.0f / curve_tessellation;
	float revolution_d = 2.0 * PI / revolution_tessellation;
	TriangleNet *triangleNet = new TriangleNet(revolution_tessellation, num_vertices / 3 * curve_tessellation);

	for (int i = 0; i < num_vertices - 3; i += 3) {
		Matrix P;
		for (int j = 0; j < 4; ++j) {
			P.Set(j, 0, vertices[i + j].x());
			P.Set(j, 1, vertices[i + j].y());
			P.Set(j, 2, vertices[i + j].z());
		}
		for (int j = 0; j <= curve_tessellation; ++j) {
			float t = curve_d * j;
			float t2 = t * t;
			float t3 = t2 * t;
			Vec4f T(t3, t2, t, 1);
			Bezier_Matrix.Transform(T);
			P.Transform(T);
			for (int k = 0; k <= revolution_tessellation; ++k) {
				float theta = revolution_d * k;
				float x = cos(theta) * T.x() + sin(theta) * T.z();
				float y = T.y();
				float z = -sin(theta) * T.x() + cos(theta) * T.z();
				triangleNet->SetVertex(k, j + (i / 3 * curve_tessellation), Vec3f(x, y, z));
			}
		}
	}
	return triangleNet;
}


void BSplineCurve::drawCurves(ArgParser *args) {
	glLineWidth(3);
	glColor3f(0, 1, 0);
	glBegin(GL_LINE_STRIP);
	int tessellation = args->curve_tessellation;
	float d = 1.0f / tessellation;
	for (int i = 0; i < num_vertices - 3; ++i) {
		Matrix P;
		for (int j = 0; j < 4; ++j) {
			P.Set(j, 0, vertices[i + j].x());
			P.Set(j, 1, vertices[i + j].y());
			P.Set(j, 2, vertices[i + j].z());
		}
		for (int j = 0; j <= tessellation; ++j) {
			float t = d * j;
			float t2 = t * t;
			float t3 = t2 * t;
			Vec4f T(t3, t2, t, 1);
			BSpline_Matrix.Transform(T);
			P.Transform(T);
			glVertex3f(T.x(), T.y(), T.z());
		}
	}
	glEnd();
}

void BSplineCurve::OutputBezier(FILE *file) {
	fprintf(file, "\n");
	fprintf(file, "bezier\nnum_vertices %d\n", num_vertices);
	Matrix P;
	for (int i = 0; i < num_vertices; ++i) {
		P.Set(i, 0, vertices[i].x());
		P.Set(i, 1, vertices[i].y());
		P.Set(i, 2, vertices[i].z());
	}
	Matrix inv_Bezier_Matrix = Bezier_Matrix;
	inv_Bezier_Matrix.Inverse();
	Matrix res = P * BSpline_Matrix * inv_Bezier_Matrix;
	for (int i = 0; i < num_vertices; ++i) {
		fprintf(file, "%f %f %f\n", res.Get(i, 0), res.Get(i, 1), res.Get(i, 2));
	}
}

void BSplineCurve::OutputBSpline(FILE *file) {
	fprintf(file, "\n");
	fprintf(file, "bspline\nnum_vertices %d\n", num_vertices);
	for (Vec3f &v : vertices) {
		fprintf(file, "%f %f %f\n", v.x(), v.y(), v.z());
	}
}

void BSplineCurve::moveControlPoint(int selectedPoint, float x, float y) {
	vertices[selectedPoint].Set(x, y, 0);
}

void BSplineCurve::addControlPoint(int selectedPoint, float x, float y) {
	num_vertices++;
	auto it = vertices.begin();
	while (selectedPoint--)
		it++;
	vertices.insert(it, Vec3f(x, y, 0));
}

void BSplineCurve::deleteControlPoint(int selectedPoint) {
	if (num_vertices <= 4)
		return;
	auto it = vertices.begin();
	while (selectedPoint--)
		it++;
	vertices.erase(it);
	num_vertices--;
}

TriangleMesh *BSplineCurve::OutputTriangles(ArgParser *args) {
	int curve_tessellation = args->curve_tessellation;
	int revolution_tessellation = args->revolution_tessellation;
	float curve_d = 1.0f / curve_tessellation;
	float revolution_d = 2.0 * PI / revolution_tessellation;
	TriangleNet *triangleNet = new TriangleNet(revolution_tessellation, (num_vertices - 3) * curve_tessellation);

	for (int i = 0; i < num_vertices - 3; i++) {
		Matrix P;
		for (int j = 0; j < 4; ++j) {
			P.Set(j, 0, vertices[i + j].x());
			P.Set(j, 1, vertices[i + j].y());
			P.Set(j, 2, vertices[i + j].z());
		}
		for (int j = 0; j <= curve_tessellation; ++j) {
			float t = curve_d * j;
			float t2 = t * t;
			float t3 = t2 * t;
			Vec4f T(t3, t2, t, 1);
			BSpline_Matrix.Transform(T);
			P.Transform(T);
			for (int k = 0; k <= revolution_tessellation; ++k) {
				float theta = revolution_d * k;
				float x = cos(theta) * T.x() + sin(theta) * T.z();
				float y = T.y();
				float z = -sin(theta) * T.x() + cos(theta) * T.z();
				triangleNet->SetVertex(k, j + (i * curve_tessellation), Vec3f(x, y, z));
			}
		}
	}
	return triangleNet;
}


void SurfaceOfRevolution::Paint(ArgParser *args) {
	curve->Paint(args);
}

TriangleMesh *SurfaceOfRevolution::OutputTriangles(ArgParser *args) {
	return curve->OutputTriangles(args);
}

void SurfaceOfRevolution::OutputBSpline(FILE *file) {
	curve->OutputBSpline(file);
}

void SurfaceOfRevolution::moveControlPoint(int selectedPoint, float x, float y) {
	curve->moveControlPoint(selectedPoint, x, y);
}

void SurfaceOfRevolution::addControlPoint(int selectedPoint, float x, float y) {
	curve->addControlPoint(selectedPoint, x, y);
}

void SurfaceOfRevolution::deleteControlPoint(int selectedPoint) {
	curve->deleteControlPoint(selectedPoint);
}


void BezierPatch::Paint(ArgParser *args) {
	glLineWidth(5);
	glColor3f(0, 0, 1);
	glBegin(GL_LINE_STRIP);
	for (Vec3f &v : vertices)
		glVertex3f(v.x(), v.y(), v.z());
	glEnd();

	glLineWidth(3);
	glColor3f(0, 1, 0);
	glBegin(GL_POINTS);
	int patch_tessellation = args->patch_tessellation;
	float d = 1.0f / patch_tessellation;
	vector<Matrix> G(4);
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			G[i].Set(j, 0, vertices[i * 4 + j].x());
			G[i].Set(j, 1, vertices[i * 4 + j].y());
			G[i].Set(j, 2, vertices[i * 4 + j].z());
		}
	}
	for (int i = 0; i <= patch_tessellation; ++i) {
		float t = i * d;
		float t2 = t * t;
		float t3 = t2 * t;
		Matrix G_new;
		for (int j = 0; j < 4; ++j) {
			Vec4f T(t3, t2, t, 1);
			Bezier_Matrix.Transform(T);
			G[j].Transform(T);
			G_new.Set(j, 0, T.x());
			G_new.Set(j, 1, T.y());
			G_new.Set(j, 2, T.z());
		}
		for (int j = 0; j <= patch_tessellation; ++j) {
			float s = j * d;
			float s2 = s * s;
			float s3 = s2 * s;
			Vec4f S(s3, s2, s, 1);
			Bezier_Matrix.Transform(S);
			G_new.Transform(S);
			glVertex3f(S.x(), S.y(), S.z());
		}
	}
	glEnd();

	glPointSize(5);
	glColor3f(1, 0, 0);
	for (int i = 0; i < 16; i += 4) {
		glBegin(GL_POINTS);
		glVertex3f(vertices[i].x(), vertices[i].y(), vertices[i].z());
		glVertex3f(vertices[i + 1].x(), vertices[i + 1].y(), vertices[i + 1].z());
		glVertex3f(vertices[i + 2].x(), vertices[i + 2].y(), vertices[i + 2].z());
		glVertex3f(vertices[i + 3].x(), vertices[i + 3].y(), vertices[i + 3].z());
		glEnd();
	}
}

TriangleMesh *BezierPatch::OutputTriangles(ArgParser *args) {
	int patch_tessellation = args->patch_tessellation;
	float d = 1.0f / patch_tessellation;
	TriangleNet *triangleNet = new TriangleNet(patch_tessellation, patch_tessellation);
	vector<Matrix> G(4);
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			G[i].Set(j, 0, vertices[i * 4 + j].x());
			G[i].Set(j, 1, vertices[i * 4 + j].y());
			G[i].Set(j, 2, vertices[i * 4 + j].z());
		}
	}
	for (int i = 0; i <= patch_tessellation; ++i) {
		float t = i * d;
		float t2 = t * t;
		float t3 = t2 * t;
		Matrix G_new;
		for (int j = 0; j < 4; ++j) {
			Vec4f T(t3, t2, t, 1);
			Bezier_Matrix.Transform(T);
			G[j].Transform(T);
			G_new.Set(j, 0, T.x());
			G_new.Set(j, 1, T.y());
			G_new.Set(j, 2, T.z());
		}
		for (int j = 0; j <= patch_tessellation; ++j) {
			float s = j * d;
			float s2 = s * s;
			float s3 = s2 * s;
			Vec4f S(s3, s2, s, 1);
			Bezier_Matrix.Transform(S);
			G_new.Transform(S);
			triangleNet->SetVertex(i, j, Vec3f(S.x(), S.y(), S.z()));
		}
	}
	return triangleNet;
}






















#pragma once

#include "matrix.h"
#include "triangle_mesh.h"
#include "arg_parser.h"
#include <vector>

class Spline {
public:
	Spline(){}

	//SET
	virtual void set(int i, Vec3f v){}

	// FOR VISUALIZATION
	virtual void Paint(ArgParser *args) = 0;

	// FOR CONVERTING BETWEEN SPLINE TYPES
	virtual void OutputBezier(FILE *file) {}
	virtual void OutputBSpline(FILE *file){}

	// FOR CONTROL POINT PICKING
	virtual int getNumVertices() = 0;
	virtual Vec3f getVertex(int i) = 0;

	// FOR EDITING OPERATIONS
	virtual void moveControlPoint(int selectedPoint, float x, float y){}
	virtual void addControlPoint(int selectedPoint, float x, float y){}
	virtual void deleteControlPoint(int selectedPoint){}

	// FOR GENERATING TRIANGLES
	virtual TriangleMesh* OutputTriangles(ArgParser *args) { return nullptr; }

protected:
	static Matrix Bezier_Matrix;
	static Matrix BSpline_Matrix;
};


class Curve :public Spline {
protected:
	int num_vertices;
	vector<Vec3f> vertices;

	void drawLines();
	void drawPoints();
	virtual void drawCurves(ArgParser *args) = 0;

public:
	Curve(int num):num_vertices(num){
		vertices.resize(num_vertices);
	}

	void set(int i, Vec3f v) override { vertices[i] = v; }

	int getNumVertices() override { return num_vertices; }

	Vec3f getVertex(int i) override { return vertices[i]; }

	void Paint(ArgParser *args) override;		
};


class BezierCurve :public Curve
{
private:
	void drawCurves(ArgParser *args)override;

public:
	BezierCurve(int num):Curve(num){}

	void OutputBezier(FILE *file) override;

	void OutputBSpline(FILE *file) override;

	void moveControlPoint(int selectedPoint, float x, float y) override;

	void addControlPoint(int selectedPoint, float x, float y) override;

	void deleteControlPoint(int selectedPoint) override;

	TriangleMesh *OutputTriangles(ArgParser *args) override;
};


class BSplineCurve : public Curve {
public:
	BSplineCurve(int num) : Curve(num) {}

	void OutputBezier(FILE *file) override;

	void OutputBSpline(FILE *file) override;

	void moveControlPoint(int selectedPoint, float x, float y) override;

	void addControlPoint(int selectedPoint, float x, float y) override;

	void deleteControlPoint(int selectedPoint) override;

	TriangleMesh *OutputTriangles(ArgParser *args) override;

private:
	void drawCurves(ArgParser *args) override;
};


class Surface : public Spline {
public:
	Surface() {}

};


class SurfaceOfRevolution : public Surface {
public:
	SurfaceOfRevolution(Curve *c) : curve(c) {}

	void set(int i, Vec3f v) override { curve->set(i, v); }

	int getNumVertices() override { return curve->getNumVertices(); }

	Vec3f getVertex(int i) override { return curve->getVertex(i); }

	void Paint(ArgParser *args) override;

	void moveControlPoint(int selectedPoint, float x, float y) override;

	void addControlPoint(int selectedPoint, float x, float y) override;

	void deleteControlPoint(int selectedPoint) override;

	TriangleMesh *OutputTriangles(ArgParser *args) override;

	void OutputBSpline(FILE *file) override;

private:
	Curve *curve;
};

class BezierPatch : public Surface {
public:
	BezierPatch() {
		num_vertices = 16;
		vertices.resize(num_vertices);
	}

	void set(int i, Vec3f v) override { vertices[i] = v; }

	int getNumVertices() override { return num_vertices; }

	Vec3f getVertex(int i) override { return vertices[i]; }

	void Paint(ArgParser *args) override;

	TriangleMesh *OutputTriangles(ArgParser *args) override;

private:
	int num_vertices;
	std::vector<Vec3f> vertices;
};











#pragma once
#ifndef _MATH3D_H_
#define _MATH3D_H_

// #ifndef INIFINITY
// #define INIFINITY      1.1754E+38F
// #endif

class Vector3D
{
public:
	Vector3D();
	Vector3D(float x, float y);
	Vector3D(float x, float y, float z);
	Vector3D(unsigned int *v);
	Vector3D(float *v);
	virtual ~Vector3D();

	float length();
	Vector3D operator- ();
	Vector3D operator+  (const Vector3D v);
	Vector3D operator-  (const Vector3D v);
	Vector3D operator*  (const float f);
	Vector3D operator/  (const float f);
	void     operator=  (const Vector3D v);
	void     operator+= (const Vector3D v);
	void     operator-= (const Vector3D v);
	bool     operator== (const Vector3D v);
	bool     operator!= (const Vector3D v);
	float    operator[] (const int idx);
	operator float* ();

	float    dot(const Vector3D v);
	Vector3D cross(const Vector3D v);
	Vector3D normalize();

	virtual float x();
	virtual float y();
	virtual float z();

	virtual void x(float val);
	virtual void y(float val);
	virtual void z(float val);

private:
	float x_val;
	float y_val;
	float z_val;
};

#endif

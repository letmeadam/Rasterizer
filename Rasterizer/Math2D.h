// #pragma once
#ifndef _MATH2D_H_
#define _MATH2D_H_

#include "Math3D.h"
// #ifndef INIFINITY
// #define INIFINITY      1.1754E+38F
// #endif

class Vector2D : public Vector3D
{
public:
	Vector2D();
	Vector2D(float x, float y);
	Vector2D(float *v);
	virtual ~Vector2D();

	void     operator=  (const Vector3D v);

	// float length();
	// Vector2D operator- ();
	// Vector2D operator+  (const Vector2D v);
	// Vector2D operator-  (const Vector2D v);
	// void     operator=  (const Vector2D v);
	// void     operator+= (const Vector2D v);
	// void     operator-= (const Vector2D v);
	// bool     operator== (const Vector2D v);
	// bool     operator!= (const Vector2D v);
	// float    operator[] (const int idx);

	// float    dot  (const Vector2D v);
	// Vector2D cross(const Vector2D v);

	float x();
	float y();
	float z();

	void x(float val);
	void y(float val);
	void z(float val);

	// private:
	// float x_val;
	// float y_val;
};

#endif

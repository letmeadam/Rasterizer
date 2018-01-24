#include "stdafx.h"

#include "Math2D.h"
#include <cmath>

using namespace std;

Vector2D::Vector2D() {
	this->x(0);
	this->y(0);
}

Vector2D::Vector2D(float x, float y) {
	this->x(x);
	this->y(y);
	// x_val = x;
	// y_val = y;
	// z_val = 0;
}

Vector2D::Vector2D(float *v) {
	this->x(v[0]);
	this->y(v[1]);
}

Vector2D::~Vector2D() {}

void Vector2D::operator= (Vector3D v) {
	this->x(v.x());
	this->y(v.y());
	this->z(v.z());
}

// float Vector2D::length() {
// 	return sqrt(pow(x_val, 2.0f) + pow(y_val, 2.0f));
// }

// Vector2D Vector2D::operator- () {
// 	return Vector2D(-this->x(), -this->y());
// }

// Vector2D Vector2D::operator+ (Vector2D v) {
// 	return Vector2D(this->x() + v.x(), this->y() + v.y(), this->z() + v.z());
// }

// Vector2D Vector2D::operator- (Vector2D v) {
// 	return this->Vector2D::operator+(v.Vector2D::operator-());
// }

// void Vector2D::operator= (Vector2D v) {
// 	this->x(v.x());
// 	this->y(v.y());
// }

// void Vector2D::operator+= (Vector2D v) {
// 	this->Vector2D::operator=(v.Vector2D::operator+(*this));
// }

// void Vector2D::operator-= (Vector2D v) {
// 	this->Vector2D::operator=(v.Vector2D::operator-(*this));
// }

// bool Vector2D::operator== (Vector2D v) {
// 	return (this->x_val == v.x() && this->y_val == v.y() && this->z_val == v.z());
// }

// bool Vector2D::operator!= (Vector2D v) {
// 	return !(this->operator==(v));
// }

// float Vector2D::operator[] (int idx){
// 	if (idx == 0)
// 		return this->x_val;
// 	else if (idx == 1)
// 		return this->y_val;
// 	else if (idx == 2)
// 		return this->z_val;
// 	else
// 		return 0;
// }


// float Vector2D::dot(Vector2D v) {
// 	return this->x_val * v.x() + this->y_val * v.y() + this->z_val * v.z();
// }

// Vector2D Vector2D::cross(Vector2D v) {
// 	Vector2D c_vector;
// 	c_vector.x(this->y_val * v.z() - this->z_val * v.y());
// 	c_vector.y(this->z_val * v.x() - this->x_val * v.z());
// 	c_vector.z(this->x_val * v.y() - this->y_val * v.x());
// 	return c_vector;
// }

float Vector2D::x() {
	return this->Vector3D::x();
}

float Vector2D::y() {
	return this->Vector3D::y();
	// return y_val;
}

float Vector2D::z() {
	// No Z-Value
	return 0;
}

void Vector2D::x(float val) {
	this->Vector3D::x(val);
	// x_val = val;
}

void Vector2D::y(float val) {
	this->Vector3D::y(val);
	// y_val = val;
}

void Vector2D::z(float val) {
	// No Z-Value
}

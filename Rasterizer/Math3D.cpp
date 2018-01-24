#include "stdafx.h"

#include "Math3D.h"
#include <cmath>

using namespace std;

Vector3D::Vector3D() {
	this->x(0);
	this->y(0);
	this->z(0);
}

Vector3D::Vector3D(float x, float y) {
	this->x(x);
	this->y(y);
	this->z(0);
}

Vector3D::Vector3D(float x, float y, float z) {
	this->x(x);
	this->y(y);
	this->z(z);
	// y_val = y;
	// z_val = z;
}

Vector3D::Vector3D(unsigned int *v) {
	this->x((float)v[0]);
	this->y((float)v[1]);
	this->z((float)v[2]);
}

Vector3D::Vector3D(float *v) {
	this->x(v[0]);
	this->y(v[1]);
	this->z(v[2]);
}

Vector3D::~Vector3D() {}

float Vector3D::length() {
	return sqrt(pow(this->x(), 2.0f) + pow(this->y(), 2.0f) + pow(this->z(), 2.0f));
}

Vector3D Vector3D::operator- () {
	return Vector3D(-this->x(), -this->y(), -this->z());
}

Vector3D Vector3D::operator+ (Vector3D v) {
	return Vector3D(this->x() + v.x(), this->y() + v.y(), this->z() + v.z());
}

Vector3D Vector3D::operator- (Vector3D v) {
	return this->Vector3D::operator+(v.Vector3D::operator-());
}

Vector3D Vector3D::operator* (float f) {
	return Vector3D(this->x() * f, this->y() * f, this->z() * f);
}

Vector3D Vector3D::operator/ (float f) {
	return this->Vector3D::operator*(1.0f / f);
}

void Vector3D::operator= (Vector3D v) {
	this->x(v.x());
	this->y(v.y());
	this->z(v.z());
}

void Vector3D::operator+= (Vector3D v) {
	this->Vector3D::operator=(v.Vector3D::operator+(*this));
}

void Vector3D::operator-= (Vector3D v) {
	this->Vector3D::operator=(v.Vector3D::operator-(*this));
}

bool Vector3D::operator== (Vector3D v) {
	return (this->x() == v.x() && this->y() == v.y() && this->z() == v.z());
}

bool Vector3D::operator!= (Vector3D v) {
	return !(this->operator==(v));
}

float Vector3D::operator[] (int idx) {
	if (idx == 0)
		return this->x();
	else if (idx == 1)
		return this->y();
	else if (idx == 2)
		return this->z();
	else
		return 0;
}

Vector3D::operator float* () {
	float *temp = new float[3]();
	temp[0] = this->x();
	temp[1] = this->y();
	temp[2] = this->z();
	return temp;
}


float Vector3D::dot(Vector3D v) {
	return this->x() * v.x() + this->y() * v.y() + this->z() * v.z();
}

Vector3D Vector3D::cross(Vector3D v) {
	Vector3D c_vector;
	c_vector.x(this->y() * v.z() - this->z() * v.y());
	c_vector.y(this->z() * v.x() - this->x() * v.z());
	c_vector.z(this->x() * v.y() - this->y() * v.x());
	return c_vector;
}

Vector3D Vector3D::normalize() {
	float magnitude = this->length();

	if (magnitude != 0)
		return this->Vector3D::operator/(magnitude);
	else
		return *this;
}


float Vector3D::x() {
	return x_val;
}

float Vector3D::y() {
	return y_val;
}

float Vector3D::z() {
	return z_val;
}

void Vector3D::x(float val) {
	x_val = val;
}

void Vector3D::y(float val) {
	y_val = val;
}

void Vector3D::z(float val) {
	z_val = val;
}

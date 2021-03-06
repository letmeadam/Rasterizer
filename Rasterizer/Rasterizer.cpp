#include "stdafx.h"

/* This program is expanded from the educational "Release code for program 1 CPE 471 Fall 2016" at Calpoly - San Luis Obispo */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <limits>

/*
#ifdef WIN32
	#include <windows.h>
#else
	#include "sys/time.h"
#endif
*/

#include <windows.h>
#undef min

#include <omp.h>

#define  TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Image.h"
// #include "BufObj.h"
#include "Math3D.h"
// #include "Math2D.h"

// #define CLOCKWISE_ORDERING
#define FORCE_NORMALS 0

#define BACKGROUND_RED     128
#define BACKGROUND_GREEN   128
#define BACKGROUND_BLUE    128

#define CLAMP(_num, _upper) (min(max((int) (_num), 0), (_upper)))

#define FACE_VERTICES 3
#define EPSILON       1E-5

#ifndef INIFINITY
#define INIFINITY      1.1754E+38F
#endif

#define COLOR_MAX_F 255.0f
#define COLOR_MAX   ((int) COLOR_MAX_F)

#define INTENSITY_AMBIENT  0.3f
#define INTENSITY_DIFFUSE  0.75f
#define INTENSITY_SPECULAR 0.25f

#define ALPHA 100.0f

Vector3D L_POS(0.0f, 0.0f, 4096.0f);
Vector3D L_COLOR(255.0f, 255.0f, 255.0f);

#define ANGLE (1.0f * 3.1415f * 0.25f)

#define TRANS_X    0.0f
#define TRANS_Y    0.0f
#define TRANS_Z  -15.0f

Vector3D V_POS(0.5f, 0.5f, 0.0f);

double t_start, t_end, t_setup, t_render, t_save;
unsigned long st_start, st_end;

using namespace std;

int g_width, g_height;

/* //FOR UNIX SYSTEMS
double dtime() {
	double tseconds = 0.0;
	struct timeval mytime;

	gettimeofday(&mytime, (struct timezone*) 0);
	tseconds = (double)(mytime.tv_sec + mytime.tv_usec * 1.0e-6);

	return(tseconds);
}
*/

//FOR WINDOWS SYSTEMS
unsigned long dtime() {
	unsigned long msec = 0.0;
	SYSTEMTIME st;

	GetSystemTime(&st);

	msec = st.wMilliseconds +
		   st.wSecond * 1000 + 
		   st.wMinute * 1000 * 60 + 
		   st.wHour   * 1000 * 60 * 60 +
		   st.wDay    * 1000 * 60 * 60 * 24;

	return msec;
}

/* Resizes object shapes to fit all vertices within [-1, 1] in each axis */
//NOTE: adapted from computer graphics coursework
void resize_obj(std::vector<tinyobj::shape_t> &shapes) {
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;
	float epsilon = 0.001f; //NOTE: Should change this to the #define'd EPSILON

	minX = minY = minZ =  INIFINITY;
	maxX = maxY = maxZ = -INIFINITY;

	//Go through all vertices to determine min and max of each dimension
	for (size_t i = 0; i < shapes.size(); i++) {
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			if (shapes[i].mesh.positions[3 * v + 0] < minX)
				minX = shapes[i].mesh.positions[3 * v + 0];
			if (shapes[i].mesh.positions[3 * v + 0] > maxX)
				maxX = shapes[i].mesh.positions[3 * v + 0];

			if (shapes[i].mesh.positions[3 * v + 1] < minY)
				minY = shapes[i].mesh.positions[3 * v + 1];
			if (shapes[i].mesh.positions[3 * v + 1] > maxY)
				maxY = shapes[i].mesh.positions[3 * v + 1];

			if (shapes[i].mesh.positions[3 * v + 2] < minZ)
				minZ = shapes[i].mesh.positions[3 * v + 2];
			if (shapes[i].mesh.positions[3 * v + 2] > maxZ)
				maxZ = shapes[i].mesh.positions[3 * v + 2];
		}
	}

	/* From min and max compute necessary scale and shift for each direction */

	// "extents" are calculated from (maximum's - minimum's) in each direction
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = maxX - minX;
	yExtent = maxY - minY;
	zExtent = maxZ - minZ;

	// maxExtent is selected by comparison
	if (xExtent >= yExtent && xExtent >= zExtent) {
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent) {
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent) {
		maxExtent = zExtent;
	}

	// scaling is calculated to resize mesh to a maximum of 2 units in each direction
	scaleX = scaleY = scaleZ = 1.8f / maxExtent; // 2.0f / maxExtent; Changed to 1.8f for border padding

	// shifting is calculated to center resize'd mesh about origin

	shiftX = minX + (xExtent / 2.0f);
	shiftY = minY + (yExtent / 2.0f);
	shiftZ = minZ + (zExtent / 2.0f);

	/* Go through all vertices shift and scale them */
#pragma omp parallel
	{
#pragma omp for
		for (size_t i = 0; i < shapes.size(); i++) {
#pragma omp for
			for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
				// asserts fail if vertex positions (after scaling/shifting) are out of range
				// from -1 to +1 with "epsilon" error
				shapes[i].mesh.positions[3 * v + 0] = (shapes[i].mesh.positions[3 * v + 0] - shiftX) * scaleX;
				assert(shapes[i].mesh.positions[3 * v + 0] >= -1.0 - epsilon);
				assert(shapes[i].mesh.positions[3 * v + 0] <= 1.0 + epsilon);

				shapes[i].mesh.positions[3 * v + 1] = (shapes[i].mesh.positions[3 * v + 1] - shiftY) * scaleY;
				assert(shapes[i].mesh.positions[3 * v + 1] >= -1.0 - epsilon);
				assert(shapes[i].mesh.positions[3 * v + 1] <= 1.0 + epsilon);

				shapes[i].mesh.positions[3 * v + 2] = (shapes[i].mesh.positions[3 * v + 2] - shiftZ) * scaleZ;
				assert(shapes[i].mesh.positions[3 * v + 2] >= -1.0 - epsilon);
				assert(shapes[i].mesh.positions[3 * v + 2] <= 1.0 + epsilon);
			}
		}
	}
}

/* Rotates object about X-axis using vector notation of matrix multiplication */
void rotate_x_obj(std::vector<tinyobj::shape_t> &shapes, float radians) {
	/* Rotation about X axis
	| 1    0        0    | |x| = |         x         |
	| 0 cos(rX) -sin(rX) | |y| = |ycos(rX) - zsin(rX)|
	| 0 sin(rX)  cos(rX) | |z| = |ysin(rX) + zcos(rX)|
	*/

#pragma omp parallel
	{
#pragma omp for
		for (size_t i = 0; i < shapes.size(); i++) {
			assert(shapes[i].mesh.positions.size() == shapes[i].mesh.normals.size());
#pragma omp for
			for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
				float px = shapes[i].mesh.positions[3 * v + 0];
				float py = shapes[i].mesh.positions[3 * v + 1];
				float pz = shapes[i].mesh.positions[3 * v + 2];

				shapes[i].mesh.positions[3 * v + 0] = px;
				shapes[i].mesh.positions[3 * v + 1] = py * cos(radians) - pz * sin(radians);
				shapes[i].mesh.positions[3 * v + 2] = py * sin(radians) + pz * cos(radians);
//			}
//#pragma omp for
//			for (size_t v = 0; v < shapes[i].mesh.normals.size() / 3; v++) {
				float nx = shapes[i].mesh.normals[3 * v + 0];
				float ny = shapes[i].mesh.normals[3 * v + 1];
				float nz = shapes[i].mesh.normals[3 * v + 2];

				shapes[i].mesh.normals[3 * v + 0] = nx;
				shapes[i].mesh.normals[3 * v + 1] = ny * cos(radians) - nz * sin(radians);
				shapes[i].mesh.normals[3 * v + 2] = ny * sin(radians) + nz * cos(radians);
			}
		}
	}
}

/* Rotates object about Y-axis using vector notation of matrix multiplication */
void rotate_y_obj(std::vector<tinyobj::shape_t> &shapes, float radians) {
	/* Rotation about X axis
	| cos(rY)  0  sin(rY) | |x| = |xcos(rY)  +  zsin(rY)|
	|    0     1     0    | |y| = |          x          |
	| -sin(rY) 0  cos(rY) | |z| = |-xsin(rX) +  zcos(rX)|
	*/
#pragma omp parallel
	{
		for (size_t i = 0; i < shapes.size(); i++) {
			assert(shapes[i].mesh.positions.size() == shapes[i].mesh.normals.size());
#pragma omp for
			for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
				float px = shapes[i].mesh.positions[3 * v + 0];
				float py = shapes[i].mesh.positions[3 * v + 1];
				float pz = shapes[i].mesh.positions[3 * v + 2];

				shapes[i].mesh.positions[3 * v + 0] =  px * cos(radians) + pz * sin(radians);
				shapes[i].mesh.positions[3 * v + 1] =  py;
				shapes[i].mesh.positions[3 * v + 2] = -px * sin(radians) + pz * cos(radians);
//			}
//#pragma omp for
//			for (size_t v = 0; v < shapes[i].mesh.normals.size() / 3; v++) {
				float nx = shapes[i].mesh.normals[3 * v + 0];
				float ny = shapes[i].mesh.normals[3 * v + 1];
				float nz = shapes[i].mesh.normals[3 * v + 2];

				shapes[i].mesh.normals[3 * v + 0] =  nx * cos(radians) + nz * sin(radians);
				shapes[i].mesh.normals[3 * v + 1] =  ny;
				shapes[i].mesh.normals[3 * v + 2] = -nx * sin(radians) + nz * cos(radians);
			}
		}
	}
}

/* Translates object vertices by the given Vector3D */
void translate_obj(std::vector<tinyobj::shape_t> &shapes, Vector3D trans) {
#pragma omp parallel
	{
#pragma omp for
		for (size_t i = 0; i < shapes.size(); i++) {
#pragma omp for
			for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
				shapes[i].mesh.positions[3 * v + 0] = shapes[i].mesh.positions[3 * v + 0] + trans.x();
				shapes[i].mesh.positions[3 * v + 1] = shapes[i].mesh.positions[3 * v + 1] + trans.y();
				shapes[i].mesh.positions[3 * v + 2] = shapes[i].mesh.positions[3 * v + 2] + trans.z();
			}
		}
	}
}

/* Convert 3D "WORLD" coordinates to 2D "VIEW" coordinates
* i.e. -1 -> +1 "WORLD" units transforms into
* -(min(g_width, g_height) / 2) -> +(min(g_width, g_height) / 2) "VIEW" units (aka pixels)
*
* NOTE: also centers shift/scale to center of view frustrum
*/
void map_to_2d(std::vector<tinyobj::shape_t> &shapes) {
	float scaleX, scaleY;
	float shiftX, shiftY;
	float g_left, g_right, g_top, g_bottom;

	if (g_width > g_height) { // ex: 800 x 600
		g_left   = (float)(-g_width) / g_height; // ex: -1.3333
		g_right  = (float)  g_width  / g_height; // ex: +1.3333
		g_top    =  1.0;
		g_bottom = -1.0;
	}
	else {
		g_left   = -1.0;
		g_right  =  1.0;
		g_top    = (float)  g_height  / g_width;
		g_bottom = (float)(-g_height) / g_width;
	}

	scaleX = (g_width - 1) / (g_right - g_left); // ex: (800 - 1) / (1.3333 - -1.3333) = (799 / 2.6666) = 299.625
	shiftX = g_left * (-scaleX); // ex: -1.3333 * -299.625 = 399.5

	scaleY = (g_height - 1) / (g_top - g_bottom); // ex: (600 - 1) / (1.0 - -1.0) = (599 / 2) = 299.5
	shiftY = g_bottom * (-scaleY); // ex: -1.0 * -299.5 = 299.5

#pragma omp parallel
	{
#pragma omp for
		for (size_t i = 0; i < shapes.size(); i++) {
#pragma omp for
			for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
				shapes[i].mesh.positions[3 * v + 0] = shapes[i].mesh.positions[3 * v + 0] * scaleX + shiftX;
				shapes[i].mesh.positions[3 * v + 1] = shapes[i].mesh.positions[3 * v + 1] * scaleY + shiftY;
			}
		}
	}

	// Transform LIGHT position from WORLD coord's' to VIEW coord's 
	L_POS.x(L_POS.x() * scaleX);
	L_POS.x(L_POS.x() + shiftX);
	L_POS.y(L_POS.y() * scaleY);
	L_POS.y(L_POS.y() + shiftY);

	V_POS.x(V_POS.x() * scaleX);
	V_POS.x(V_POS.x() + shiftX);
	V_POS.y(V_POS.y() * scaleY);
	V_POS.y(V_POS.y() + shiftY);
	// NOTE: Alternative? V_POS = V_POS * Vector3D(scaleX, scaleY, 1.0f) + Vector3D(shiftX, shiftY, 0.0f);

	// L_POS.x((g_width  - 1) / 2.0f);
	// L_POS.y((g_height - 1) / 2.0f);
}

//NOTE: Could try a #define or inline function to optimize?

// Calculate Minimum of three floats
float Min(float A, float B, float C) {
	float min = A;
	if (B < min)
		min = B;
	if (C < min)
		min = C;
	return min;
}

// Calculate Maximum of three floats
float Max(float A, float B, float C) {
	float max = A;
	if (B > max)
		max = B;
	if (C > max)
		max = C;
	return max;
}

//IMPORTANT PERSONAL NOTE: This doesn't use 3D vectors, it only uses 2D vectors... for a 3D space... YUCK!
// Calculates area of triangle given 2D coordinates
float TriArea(Vector3D v1, Vector3D v2, Vector3D v3) {
	/* Triangle Area (2D Space, it's simpler than 3D, but :/  )):
	*          | x1 y1 1 |
	* 0.5 * Det| x2 y2 1 | = 0.5 [ x1(y2 - y3) - y1(x2 - x3) + 1(x2y3 - x3y2) ]
	*          | x3 y3 1 | = 0.5 [ x1y2 - x1y3 - x2y1 + x3y1 + x2y3 - x3y2]
	*                      = 0.5 [ (x1 - x3)(y2 - y3) - (x2 - x3)(y1 - y3) ]
	*/

	Vector3D vA(v1.x(), v1.y());
	Vector3D vB(v2.x(), v2.y());
	Vector3D vC(v3.x(), v3.y());

	return ((v1.x() - v3.x()) * (v2.y() - v3.y()) - (v2.x() - v3.x()) * (v1.y() - v3.y())) / 2.0f;
}

//Renders individual triangle given shape info, z-buffer, and vertex indices
void Render_tri(vector<tinyobj::shape_t>    &shapes,
	vector<tinyobj::material_t> &materials,
	shared_ptr<Image> img, float *depth_map,
	int shape_id, int face_id) {
	static int tri_num = 0;
	float area;
	float vA, vB, vC;

	vA = shapes[shape_id].mesh.indices[face_id * 3 + 0];
	vB = shapes[shape_id].mesh.indices[face_id * 3 + 1];
	vC = shapes[shape_id].mesh.indices[face_id * 3 + 2];

	float minX, maxX;
	float minY, maxY;

	//Face positions (of vertices)
	Vector3D v1(&(shapes[shape_id].mesh.positions[(vA * 3)]));
	assert(v1.x() == shapes[shape_id].mesh.positions[(vA * 3) + 0]);
	assert(v1.y() == shapes[shape_id].mesh.positions[(vA * 3) + 1]);
	assert(v1.z() == shapes[shape_id].mesh.positions[(vA * 3) + 2]);

	Vector3D v2(&(shapes[shape_id].mesh.positions[(vB * 3)]));
	assert(v2.x() == shapes[shape_id].mesh.positions[(vB * 3) + 0]);
	assert(v2.y() == shapes[shape_id].mesh.positions[(vB * 3) + 1]);
	assert(v2.z() == shapes[shape_id].mesh.positions[(vB * 3) + 2]);

	Vector3D v3(&(shapes[shape_id].mesh.positions[(vC * 3)]));
	assert(v3.x() == shapes[shape_id].mesh.positions[(vC * 3) + 0]);
	assert(v3.y() == shapes[shape_id].mesh.positions[(vC * 3) + 1]);
	assert(v3.z() == shapes[shape_id].mesh.positions[(vC * 3) + 2]);

	//Face normals (per vertex)
	Vector3D nA(&(shapes[shape_id].mesh.normals[(vA * 3)]));
	Vector3D nB(&(shapes[shape_id].mesh.normals[(vB * 3)]));
	Vector3D nC(&(shapes[shape_id].mesh.normals[(vC * 3)]));

	// Calculate triangle area and min/max for X/Y directions
	minX = Min(v1.x(), v2.x(), v3.x());
	maxX = Max(v1.x(), v2.x(), v3.x());

	minY = Min(v1.y(), v2.y(), v3.y());
	maxY = Max(v1.y(), v2.y(), v3.y());

	area = TriArea(v1, v2, v3);

/*
 * IMPORTANT NOTE: cannot parrallelize this section due to data dependency on 
 * writing/updating image array. Rather than creating local sections of the 
 * image and using much more memory, could try utilizing critical regions (or 
 * locks) when checking and writing to image array.
 */

	/* Go through bounding box of triangle face and update image */
	for (int x = (floor(minX) < 0 ? 0 : floor(minX)); x < ceil(maxX) && x < g_width; x++) {
		for (int y = (floor(minY) < 0 ? 0 : floor(minY)); y < ceil(maxY) && y < g_height; y++) {
			float alpha, beta, gamma;
			float z_pos;

			Vector3D pos(x, y, 0.0f);

			alpha = TriArea(v2, v3, pos) / area; //ratio of area belonging to v1
			beta  = TriArea(v3, v1, pos) / area; //ratio of area belonging to v2
			gamma = TriArea(v1, v2, pos) / area; //ratio of area belonging to v3

			// interpolate z value based on alpha, beta, and gamma ratios
			z_pos = (alpha * v1.z()) + (beta * v2.z()) + (gamma * v3.z());

			// interpolate normal based on alpha, beta, and gamma ratios
			Vector3D normal(nA * alpha + nB * beta + nC * gamma);
			normal = normal.normalize();

			// if alpha, beta, and gamma, are all positive (with EPSILON margin of error)
			if (alpha > -EPSILON && beta > -EPSILON && gamma > -EPSILON) {
				//IMPORTANT: top-left origin coordinate system used
				int idx = y * g_width + x;

				// Depth Test in Z direction
				if (z_pos > depth_map[idx]) {
					// update depth map
					depth_map[idx] = z_pos;

					// update image
					Vector3D light = (L_POS - Vector3D(x, y, z_pos)).normalize();

					Vector3D reflection = (normal * max(light.dot(normal), 0.0f) * 2.0f) - light;
					reflection = reflection.normalize();

					// Viewer/Camera positioned at center of image (offset +10.0f in the +Z direction)
					Vector3D viewer = V_POS - Vector3D(x, y, z_pos);
					viewer = viewer.normalize();

					Vector3D color;

					if (!materials.empty() && materials.size() > 0) {
						tinyobj::material_t mat = materials[shapes[shape_id].mesh.material_ids[face_id]];

						Vector3D ambient(mat.ambient);
						Vector3D diffuse(mat.diffuse);
						Vector3D specular(mat.specular);
						float shininess = mat.shininess;

						// Scale .mtl range of 0->1.0f to 0->255.0f
						ambient  = ambient  * COLOR_MAX_F;
						diffuse  = diffuse  * COLOR_MAX_F;
						specular = specular * COLOR_MAX_F;

						color = ambient * INTENSITY_AMBIENT + diffuse * max(light.dot(normal), 0.0f) * INTENSITY_DIFFUSE +
							specular * pow(max(reflection.dot(viewer), 0.0f), shininess * COLOR_MAX_F) * INTENSITY_SPECULAR;
					}
					else {
						L_COLOR = Vector3D(255.0f, 255.0f, 255.0f); // normal * fabs(light.dot(normal)) * COLOR_MAX_F;

						color = Vector3D(1.0f, 1.0f, 1.0f) * INTENSITY_AMBIENT + L_COLOR * max(light.dot(normal), 0.0f) * INTENSITY_DIFFUSE +
							L_COLOR * pow(max(reflection.dot(viewer), 0.0f), ALPHA) * INTENSITY_SPECULAR;
					}

					img->setPixel(x, y,
						CLAMP(color[0], COLOR_MAX),
						CLAMP(color[1], COLOR_MAX),
						CLAMP(color[2], COLOR_MAX));
				}
			}
		}
	}
	tri_num++;
}

void setupNormals(tinyobj::shape_t *s) {
	//NOTE: Wavefront objects (.obj's) are defaulted to counter-clockwise vertex order
	int i;

	s->mesh.normals.clear();

	// Allocate space for normal x, y, and z floats
	for (i = 0; i < s->mesh.positions.size(); i++)
		s->mesh.normals.push_back(0.0f);

	for (i = 0; i < s->mesh.indices.size() / 3; i++) {
		int idx1 = s->mesh.indices[i * 3 + 0];
		int idx2 = s->mesh.indices[i * 3 + 1];
		int idx3 = s->mesh.indices[i * 3 + 2];

		Vector3D v1(&(s->mesh.positions[idx1 * 3]));
		Vector3D v2(&(s->mesh.positions[idx2 * 3]));
		Vector3D v3(&(s->mesh.positions[idx3 * 3]));

		Vector3D u = v2 - v1;
		Vector3D v = v3 - v1;

#ifdef CLOCKWISE_ORDERING
		Vector3D c = v.cross(u);
#else
		Vector3D c = u.cross(v);
#endif

		s->mesh.normals[idx1 * 3 + 0] += c.x();
		s->mesh.normals[idx1 * 3 + 1] += c.y();
		s->mesh.normals[idx1 * 3 + 2] += c.z();

		s->mesh.normals[idx2 * 3 + 0] += c.x();
		s->mesh.normals[idx2 * 3 + 1] += c.y();
		s->mesh.normals[idx2 * 3 + 2] += c.z();

		s->mesh.normals[idx3 * 3 + 0] += c.x();
		s->mesh.normals[idx3 * 3 + 1] += c.y();
		s->mesh.normals[idx3 * 3 + 2] += c.z();
	}

	for (i = 0; i < s->mesh.normals.size() / 3; i++) {
		Vector3D n(&(s->mesh.normals[i * 3]));
		n = n.normalize();

		s->mesh.normals[i * 3 + 0] = n.x();
		s->mesh.normals[i * 3 + 1] = n.y();
		s->mesh.normals[i * 3 + 2] = n.z();
	}
}

int main(int argc, char **argv)
{
	//double t_overall_start = dtime();
	if (argc < 5) {
		//cout << argc << endl;
		cout << "\nUsage: " << argv[0] << " imageFile imageWidth imageHeight meshFile [matFile]" << endl;
		cout << "\t( i.e. " << argv[0] << " output.png 512 512 bunny.obj [bunny.mtl] )" << endl;
		return 0;
	}

	// Argument 1 - Output Image Filename
	string imgName(argv[1]);

	// Argument 2 & 3 - Image Width & Height
	g_width  = atoi(argv[2]);
	g_height = atoi(argv[3]);

	// Argument 4 - Object Filename (.obj)
	string meshName(argv[4]);

	// Argument 5 (OPTIONAL) - Material Filename (.mtl)
	// NOTE: should use getopt.h for better, optional command line features
	string matName;
	if (argc < 6) {
		// cout << argc << endl;
		// matName = (string) NULL;
	}
	else
		matName = string(argv[5]);

	/* TIMING - Overall - start */
	//t_start = dtime();
	st_start = dtime();

	// Create an image data structure with Background color
	auto image = make_shared<Image>(g_width, g_height);
	image->setColor(BACKGROUND_RED, BACKGROUND_GREEN, BACKGROUND_BLUE);

	// Create a depth map for image
	float *depth_map = (float*) malloc(g_width * g_height * sizeof(float));
	if (depth_map == NULL) {
		perror("Depth Map allocation error");
		return 0;
	}

#pragma omp parallel for
	for (int i = 0; i < g_width * g_height; i++)
		depth_map[i] = std::numeric_limits<int>::min();

	// Triangle buffer, alias for shapes[#].mesh.indices (std::vector)
	vector<unsigned int> triBuf;

	// Position buffer, alias for shapes[#].mesh.positions (std::vector)
	// vector<float> posBuf;

	// Major buffers to hold geometry and material information.
	vector<tinyobj::shape_t> shapes; // geometry
	vector<tinyobj::material_t> objMaterials; // material

	// Error Checking variables
	string errStr;
	bool rc;

	// Load object based on Material information optionally provided
	//NOTE: should test with a (const char*) NULL to clear if-statement
	if (argc < 6) {
		// Attempt to read object without a Material file (.mtl)
		rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	}
	else {
		// Attempt to read object with a Material file (.mtl)
		rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str(), matName.c_str());
	}

	if (!rc) {
		// Error checking on object read
		cerr << errStr << endl;
	}

	cout << "======= Object Information =======" << endl;
	cout << "\tNumber of shapes:    " << shapes.size() << endl;
	cout << "\tNumber of materials: " << objMaterials.size() << endl;
	for (int i = 0; i < objMaterials.size(); i++) {
		cout << "\t\tMaterial " << i << " -> " << objMaterials[i].name << endl;
		cout << "\t\t\tAmbient:  " << objMaterials[i].ambient[0]  << "\t" << objMaterials[i].ambient[1]  << "\t" << objMaterials[i].ambient[2]  << endl;
		cout << "\t\t\tDiffuse:  " << objMaterials[i].diffuse[0]  << "\t" << objMaterials[i].diffuse[1]  << "\t" << objMaterials[i].diffuse[2]  << endl;
		cout << "\t\t\tSpecular: " << objMaterials[i].specular[0] << "\t" << objMaterials[i].specular[1] << "\t" << objMaterials[i].specular[2] << endl;
	}
	cout << endl;

	for (int i = 0; i < shapes.size(); i++) {
		cout << "\t------- Shape " << i << " -------" << endl;
		cout << "\t\tNumber of triangles: " << shapes[i].mesh.indices.size() / 3 << endl;
		cout << "\t\tNumber of vertices:  " << shapes[i].mesh.positions.size() / 3 << endl;
		cout << "\t\tChecking Normals..." << endl;
		if (shapes[i].mesh.normals.size() == 0 || FORCE_NORMALS) {
			cout << "\t\t\tCreating Normals!" << endl;
			setupNormals(&shapes[i]);
		}
		else
			cout << "\t\t\tUsing Original Normals!" << endl;

		cout << "\t\tNumber of normals:   " << shapes[i].mesh.normals.size() / 3 << endl;
		cout << endl;
	}

	//rotate object about X axis
	rotate_x_obj(shapes, 3.14159f / 32.0f); //slight vertical tilt

	//rotate object about Y axis
	rotate_y_obj(shapes, ANGLE);

	//resize object to be within -1 -> 1
	resize_obj(shapes);

	//translate object
	translate_obj(shapes, Vector3D(TRANS_X, TRANS_Y, TRANS_Z));

	//map 3D vertex positions to 2D space
	Map_to_2d(shapes);

	/* TIMING - Render - start
	//t_end = dtime();
	// cout << t_end << endl;
	// cout << (t_end - t_start) << endl;

	//t_setup = (t_end - t_start);
	//t_start = dtime();
	*/

	// Render each triangle
	for (int s = 0; s < shapes.size(); s++) {
		triBuf = shapes[s].mesh.indices;
		// cout << "Rendering Shape # " << s << " with " << (triBuf.size() / 3) << " triangles..."<< endl;

		for (size_t t = 0; t < triBuf.size() / 3; t++) {
			// for (size_t t = 0; t < 11; t++) {
			// cout << "\tRendering Triangle # " << t << endl;
			Render_tri(shapes, objMaterials, image, depth_map, s, t);
		}

		// cout << "Shape # " << s << " DONE!"<< endl;
	}

	/* TIMING - Render - end
	//t_end = dtime();
	//t_render = (t_end - t_start);
	*/

	cout << endl << "Render Complete!" << endl;

	free(depth_map);

	/* TIMING - Image Write - start
	//t_start = dtime();
	*/

	//write out the image
	cout << "Writing to File..." << endl;
	image->writeToFile(imgName);
	cout << "File Written!" << endl;

	/* TIMING - Image Write - end
	//t_end = dtime();
	//t_save = (t_end - t_start);
	*/

	cout << endl;

	/* TIMING PRINT
	//printf("T_setup:  \t%12.9lf sec\n", t_setup);
	//printf("T_render: \t%12.9lf sec\n", t_render);
	//printf("T_save:   \t%12.9lf sec\n", t_save);
	//printf("\nT_total:  \t%12.9lf sec\n", dtime() - t_overall_start);
	// cout << "T_render:\t" << (t_render) << "\tusec" << endl;
	*/

	st_end = dtime();

	cout << "Total Time: " << (st_end - st_start) << " ms" << endl;

	return 0;
}

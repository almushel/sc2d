// Simple Collision 2D
// A single header 2D collision library

#ifndef SIMPLE_COLLISION_2D_H

#define SIMPLE_COLLISION_2D_H 
#include <stdbool.h>

bool sc2d_check_point_circle(float px, float py, float cx, float xy, float cr, float* overlap_x, float* overlap_y);
bool sc2d_check_point_rect(float px, float py, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y);
bool sc2d_check_circles(float p1x, float p1y, float r1, float p2x, float p2y, float r2, float* overlap_x, float* overlap_y);
bool sc2d_check_rects(float p1x, float p1y, float r1w, float r1h, float p2x, float p2y, float r2w, float r2h, float* overlap_x, float* overlap_y);
bool sc2d_check_circle_centered_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y);
bool sc2d_check_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y);

bool sc2d_check_poly2d(	float p1x, float p1y, float* p1_verts, int p1_count, 
							float p2x, float p2y, float* p2_verts, int p2_count, 
							float* overlap_x, float* overlap_y);
bool sc2d_check_point_poly2d(float px, float py, float* poly_verts, int vert_count);
bool sc2d_check_point_line(float px, float py, float start_x, float start_y, float end_x, float end_y, bool segment);

#endif

#ifdef SIMPLE_COLLISION_2D_IMPLEMENTATION

#include <math.h>

// Check for collion between a point and a circle and return penetration by reference
bool sc2d_check_point_circle(float px, float py, float cx, float cy, float cr, float* overlap_x, float* overlap_y) {
	bool result = false;

	float delta_x = cx - px;
	float delta_y = cy - py;

	float delta_m = hypotf(delta_x, delta_y);
	float delta_r =  cr - delta_m;

	if (result = delta_r > 0) {
		*overlap_x = (delta_x / delta_m) * delta_r;
		*overlap_y = (delta_y / delta_m) * delta_r;
	}

	return result;
}

// Check for collision between a point and a rectangle (left x, top y, width, height) and return penetration by reference
bool sc2d_check_point_rect(float px, float py, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	float rect_center_width = rw/2.0f;
	float rect_center_height = rh/2.0f;

	float rect_center_x = rx + rect_center_width;
	float rect_center_y = ry + rect_center_height;

	float delta_x = rect_center_x - px;
	float delta_y = rect_center_y - py;

	if (result = (fabsf(delta_x) < rect_center_width && fabsf(delta_y) < rect_center_height) ) {
		
		*overlap_x = (rect_center_width - fabsf(delta_x));
		*overlap_y = (rect_center_height - fabsf(delta_y));

		*overlap_x *= (float)(int)(*overlap_x < *overlap_y);
		*overlap_y *= (float)(int)(*overlap_x == 0);

		if (delta_y < 0) *overlap_y *= -1;
	}

	return result;
}

// Check for collion between two circles and return overlap by reference
bool sc2d_check_circles(float p1x, float p1y, float r1, float p2x, float p2y, float r2, float* overlap_x, float* overlap_y) {
	bool result = false;

	float delta_x = p2x - p1x;
	float delta_y = p2y - p1y;
	float magnitude = hypotf(delta_x, delta_y);
	float overlap_magnitude = (r1 + r2) - magnitude;

	if (result = overlap_magnitude > 0) {
		*overlap_x = (delta_x / magnitude) * overlap_magnitude;
		*overlap_y = (delta_y / magnitude) * overlap_magnitude;
	}

	return result;
}

//Check for collision between to rectangles (left x, top y, width, height) and return overlap by reference
bool sc2d_check_rects(float p1x, float p1y, float r1w, float r1h, float p2x, float p2y, float r2w, float r2h, float* overlap_x, float* overlap_y) {
	bool result = false;

	if (p1x < p2x) 	*overlap_x = (p1x + r1w) - p2x;
	else 			*overlap_x = (p2x + r2w) - p1x;

	if (p1y < p2y) 	*overlap_y = (p1y + r1h) - p2y;
	else			*overlap_y = (p2y + r2h) - p1y;
	
	if ( result = (*overlap_x > 0 && *overlap_y > 0) ) {
		if (p2x < p1x) *overlap_x *= -1;
		if (p2y < p1y) *overlap_y *= -1;

		*overlap_x *= (float)(int)(fabsf(*overlap_x) < fabsf(*overlap_y));
		*overlap_y *= (float)(int)(*overlap_x == 0);
	}

	return result;
}

// Check for collision of circle and centered rectangle (center x, center y, width, height) and return overlap by reference
// TO-DO: Handle containment (circle inside of rect)
bool sc2d_check_circle_centered_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	// Get a vector pointing from center of rect to center of circle
	float delta_x = cx - rx;
	float delta_y = cy - ry;

	if ( fabsf(delta_x) > (cr + rw) || fabsf(delta_y) > (cr + rh)) {
		result = false;
	} else {
		//Get intersection point of vector and nearest rect edge relative to the center of the rectangle
		float clamp_x = fminf(fmaxf(delta_x, -rw), rw);
		float clamp_y = fminf(fmaxf(delta_y, -rh), rh);
	
		// Get a vector pointing from the center of circle to center of rect
		delta_x = rx - cx;
		delta_y = ry - cy;

		// Get vector pointing from circle center to collision point
		clamp_x += delta_x;
		clamp_y += delta_y;

		float magnitude = hypotf(clamp_x, clamp_y);

		if (magnitude == 0.0f) magnitude = 1.0f; //Hack to avoid divide by zero when circle is inside rect
		if (magnitude < cr) {
			*overlap_x = (clamp_x / magnitude) * (cr - magnitude); 
			*overlap_y = (clamp_y / magnitude) * (cr - magnitude); 
			result = true;
		}
	}

	return result;
}

// Check for collision between circle and rectangle (left x, top y, width, height) and return overlap vector by reference
bool sc2d_check_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	rw /= 2.0f;
	rh /= 2.0f;

	rx += rw;
	ry += rh;

	result = sc2d_check_circle_centered_rect(cx, cy, cr, rx, ry, rw, rh, overlap_x, overlap_y);

	return result;
}

// Project all points in polygon to 2D vector axis (dot product)
static inline void project_poly2d_to_axis(float axis_x, float axis_y, float* poly_verts, int poly_vert_count, float* min, float* max) {
	*min=0; *max=0;

#ifndef SIMPLE_COLLISION_2D_VECTOR2
	typedef struct sc2d_v2 {float x, y;} sc2d_v2;
#endif

	sc2d_v2* v2_verts = (sc2d_v2*)poly_verts;
	
	for (int i = 0; i < poly_vert_count; i++) {
		float dot = (axis_x * v2_verts[i].x) + (axis_y * v2_verts[i].y); // dot product
		*min = fminf(*min, dot);
		*max = fmaxf(*max, dot);
	}
}

// Get vector from start index to next vertex in polygon

static inline void get_poly2d_edge(float* poly_verts, int vert_count, int start_index, float* edge_x, float* edge_y) {
#ifndef SIMPLE_COLLISION_2D_VECTOR2
	typedef struct sc2d_v2 {float x, y;} sc2d_v2;
#endif
	sc2d_v2* v2_verts = (sc2d_v2*)poly_verts;
	int end_index = (start_index + 1) % vert_count; // wrap to first vertex

	*edge_x = v2_verts[end_index].x - v2_verts[start_index].x;
	*edge_y = v2_verts[end_index].y - v2_verts[start_index].y;
}

// Get clockwise or counterclockwise normal of 2D vector
static void v2_normal(float* x, float* y, bool clockwise) {
	float temp;

	temp = *x;
	*x   = *y    * (float)(1 - (2 * (int)(!clockwise)));
	*y   = temp  * (float)(1 - (2 * (int)(clockwise) ));
}

// Normalize 2d vector
static void v2_normalize(float* x, float* y) {
	float magnitude;

	magnitude = hypotf(*x, *y);
	*x /= magnitude;
	*y /= magnitude;
}

// Check for collision between two convex polygons and return shortest axis overlap by reference
// p1_count and p2_count: The number of x/y pairs (or custom sc2d_v2 structs) in poly_verts
//
// Structs with format other than {float x,y} can be supported by defining a custom sc2d_v2 type and
// setting SIMPLE_COLLISION_2D_VECTOR2
// e.g.:
// #ifndef SIMPLE_COLLISION_2D_VECTOR2
// typedef struct v3 {float x, y, z} sc2d_v2;
// #define SIMPLE_COLLISION_2D_VECTOR2
// #endif
bool sc2d_check_poly2d(	float p1x, float p1y, float* p1_verts, int p1_count, 
						float p2x, float p2y, float* p2_verts, int p2_count, 
						float* overlap_x, float* overlap_y) {
	
	bool result = true;
	float p1_min, p1_max, p2_min, p2_max;
	float axis_x, axis_y;

	float delta_x = p2x - p1x;
	float delta_y = p2y - p1y;
	float offset = 0;
	float min_distance = INFINITY;

	// First polygon
	for (int i = 0; i < p1_count; i++) {
		get_poly2d_edge(p1_verts, p1_count, i, &axis_x, &axis_y); // Get vector pointing from curent vertex to next vertex (edge)
		v2_normal(&axis_x, &axis_y, false); // Get the normal of the edge (vector perpendicular to the edge)
		v2_normalize(&axis_x, &axis_y);
		offset = (axis_x * delta_x) + (axis_y * delta_y); // project the the vector between polygon positions to the axis (dot product)

		project_poly2d_to_axis(axis_x, axis_y, p1_verts, p1_count, &p1_min, &p1_max); // project ever y vertex in first polygon to current axis
		project_poly2d_to_axis(axis_x, axis_y, p2_verts, p2_count, &p2_min, &p2_max); // project ever y vertex in scond polygon to current axis
		
		p1_min -= offset; // Add position offset to projection
		p1_max -= offset;
		
		if ( (p1_min > p2_max) || (p1_max < p2_min)) { // If the ranges do not overlap, polygons are not touching
			return false;
		}
		
		float distance = fminf(p1_max, p2_max) - fmaxf(p1_min, p2_min);
		if (distance < min_distance) { // Update minimum distance for overlap
			min_distance = distance;
			*overlap_x = axis_x * (float)(1 - 2 * (int)(offset < 0) );
			*overlap_y = axis_y * (float)(1 - 2 * (int)(offset < 0) );
		}
	}

	// Project all vertices to all axes of the second polygon
	for (int i = 0; i < p2_count; i++) {
		get_poly2d_edge(p2_verts, p2_count, i, &axis_x, &axis_y);
		v2_normal(&axis_x, &axis_y, false);
		v2_normalize(&axis_x, &axis_y);
		offset = (axis_x * delta_x) + (axis_y * delta_y);

		project_poly2d_to_axis(axis_x, axis_y, p1_verts, p1_count, &p1_min, &p1_max);
		project_poly2d_to_axis(axis_x, axis_y, p2_verts, p2_count, &p2_min, &p2_max);
		
		p1_min -= offset;
		p1_max -= offset;
		
		if ( (p1_min > p2_max) || (p1_max < p2_min)) {
			return false;
		}

		float distance = fminf(p1_max, p2_max) - fmaxf(p1_min, p2_min);
		if (distance < min_distance) {
			min_distance = distance;
			*overlap_x = axis_x * (float)(1 - 2 * (int)(offset < 0) );
			*overlap_y = axis_y * (float)(1 - 2 * (int)(offset < 0) );
		}
	}

	*overlap_x *= min_distance;
	*overlap_y *= min_distance;

	return result;
}

// Check for collision between point and convex polygon
// poly_count: The number of x/y pairs (or custom sc2d_v2 structs) in poly_verts
bool sc2d_check_point_poly2d(float px, float py, float* poly_verts, int vert_count) {
	bool result = false;

#ifndef SIMPLE_COLLISION_2D_VECTOR2
	typedef struct sc2d_v2 {float x, y;} sc2d_v2;
#endif

	sc2d_v2* v2_verts = (sc2d_v2*)poly_verts;
	for (int i = 0, j = vert_count - 1; i < vert_count; i++) {
		if ( (v2_verts[i].y >= py) != (v2_verts[j].y >= py) &&
			 (px < (v2_verts[j].x - v2_verts[i].x) * (py - v2_verts[i].y) / (v2_verts[j].y - v2_verts[i].y) + v2_verts[i].x)
			) {
			
			result = !result;
		}

		j = i;
	}

	return result;
}

// Check for collision between point and line or line segment
bool sc2d_check_point_line(float px, float py, float start_x, float start_y, float end_x, float end_y, bool segment) {

	bool result = false;

	// Vector pointing from line start to line end
	float line_x = end_x - start_x;
	float line_y = end_y - start_y;
	float line_length = hypotf(line_y, line_x);

	// Vector point from line start to point
	float point_delta_x = px - start_x;
	float point_delta_y = py - start_y;
	float distance_to_point = hypotf(point_delta_x, point_delta_y);

	// If the angles of both vectors are equal,
	// then the point is on the ray starting at start
	if (atan2(line_y, line_x) == atan2(point_delta_y, point_delta_x)) {
		// if the distance to the point is less than or equal to line length,
		// then the point is on the segment start -> end
		if (segment && (distance_to_point <= line_length) ) {
			result = true;
		// Else start -> end describes an (infinite) line or a ray
		} else {
			result = true;
		}
	// If the angle of the line pointing in the opposite direction is equal
	// angle from start to the point,
	// then the point is still on the (infinite) line defined by start and end
	// but not the segment or ray defined by start and end
	} else if (!segment && (atan2(-line_x, -line_y) == atan2(point_delta_y, point_delta_x)) ) {
		result = true;
	}

	return result;
}

#endif
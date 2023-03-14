// Simple Collision 2D
// A single header 2D collision library

#ifndef SIMPLE_COLLISION_2D_H

#define SIMPLE_COLLISION_2D_H 
#include <stdbool.h>

#ifndef SIMPLE_COLLISION_2D_TYPES
typedef struct sc2d_v2 {float x, y;} sc2d_v2;

#define SIMPLE_COLLISION_2D_TYPES
#endif

bool sc2d_check_point_circle(sc2d_v2 p, sc2d_v2 cp, float cr, sc2d_v2* overlap);
bool sc2d_check_point_rect(sc2d_v2 p, sc2d_v2 rp, sc2d_v2 rd, sc2d_v2* overlap);
bool sc2d_check_circles(sc2d_v2 p1, float r1, sc2d_v2 p2, float r2, sc2d_v2* overlap);
bool sc2d_check_rects(sc2d_v2 p1, sc2d_v2 rd1, sc2d_v2 p2, sc2d_v2 rd2, sc2d_v2* overlap);
bool sc2d_check_circle_centered_rect(sc2d_v2 cp, float cr, sc2d_v2 rp, sc2d_v2 rd, sc2d_v2* overlap);
bool sc2d_check_circle_rect(sc2d_v2 cp, float cr, sc2d_v2 rp, sc2d_v2 rd, sc2d_v2* overlap);

bool sc2d_check_poly2d(	sc2d_v2 p1, sc2d_v2* p1_verts, int p1_count, 
							sc2d_v2 p2, sc2d_v2* p2_verts, int p2_count, 
							sc2d_v2* overlap);
bool sc2d_check_point_poly2d(sc2d_v2 p, sc2d_v2* poly_verts, int vert_count);
bool sc2d_check_point_line(sc2d_v2 p, sc2d_v2 start, sc2d_v2 end, bool segment);

#endif

#ifdef SIMPLE_COLLISION_2D_IMPLEMENTATION

#include <math.h>

// Check for collion between a point and a circle and return penetration by reference
bool sc2d_check_point_circle(sc2d_v2 p, sc2d_v2 cp, float cr, sc2d_v2* overlap) {
	bool result = false;

	float delta_x = cp.x - p.x;
	float delta_y = cp.y - p.y;

	float delta_m = hypotf(delta_x, delta_y);
	float delta_r =  cr - delta_m;

	if (result = delta_r > 0) {
		overlap->x = (delta_x / delta_m) * delta_r;
		overlap->y = (delta_y / delta_m) * delta_r;
	}

	return result;
}

// Check for collision between a point and a rectangle (left x, top y, width, height) and return penetration by reference
bool sc2d_check_point_rect(sc2d_v2 p, sc2d_v2 rp, sc2d_v2 rd, sc2d_v2* overlap) {
	bool result = false;

	float rect_center_width = rd.x/2.0f;
	float rect_center_height = rd.y/2.0f;

	float rect_center_x = rp.x + rect_center_width;
	float rect_center_y = rp.y + rect_center_height;

	float delta_x = rect_center_x - p.x;
	float delta_y = rect_center_y - p.y;

	if (result = (fabsf(delta_x) < rect_center_width && fabsf(delta_y) < rect_center_height) ) {
		
		overlap->x = (rect_center_width - fabsf(delta_x));
		overlap->y = (rect_center_height - fabsf(delta_y));

		overlap->x *= (float)(int)(overlap->x < overlap->y);
		overlap->y *= (float)(int)(overlap->x == 0);

		if (delta_y < 0) overlap->y *= -1;
	}

	return result;
}

// Check for collion between two circles and return overlap by reference
bool sc2d_check_circles(sc2d_v2 p1, float r1, sc2d_v2 p2, float r2, sc2d_v2* overlap) {
	bool result = false;

	float delta_x = p2.x - p1.x;
	float delta_y = p2.y - p1.y;
	float magnitude = hypotf(delta_x, delta_y);
	float overlap_magnitude = (r1 + r2) - magnitude;

	if (result = overlap_magnitude > 0) {
		overlap->x = (delta_x / magnitude) * overlap_magnitude;
		overlap->y = (delta_y / magnitude) * overlap_magnitude;
	}

	return result;
}

//Check for collision between to rectangles (left x, top y, width, height) and return overlap by reference
bool sc2d_check_rects(sc2d_v2 p1, sc2d_v2 rd1, sc2d_v2 p2, sc2d_v2 rd2, sc2d_v2* overlap) {
	bool result = false;

	if (p1.x < p2.x) 	overlap->x = (p1.x + rd1.x) - p2.x;
	else 			overlap->x = (p2.x + rd2.x) - p1.x;

	if (p1.y < p2.y) 	overlap->y = (p1.y + rd1.y) - p2.y;
	else			overlap->y = (p2.y + rd2.y) - p1.y;
	
	if ( result = (overlap->x > 0 && overlap->y > 0) ) {
		if (p2.x < p1.x) overlap->x *= -1;
		if (p2.y < p1.y) overlap->y *= -1;

		overlap->x *= (float)(int)(fabsf(overlap->x) < fabsf(overlap->y));
		overlap->y *= (float)(int)(overlap->x == 0);
	}

	return result;
}

// Check for collision of circle and centered rectangle (center x, center y, width, height) and return overlap by reference
// TO-DO: Handle containment (circle inside of rect)
bool sc2d_check_circle_centered_rect(sc2d_v2 cp, float cr, sc2d_v2 rp, sc2d_v2 rd, sc2d_v2* overlap) {
	bool result = false;

	// Get a vector pointing from center of rect to center of circle
	float delta_x = cp.x - rp.x;
	float delta_y = cp.y - rp.y;

	if ( fabsf(delta_x) > (cr + rd.x) || fabsf(delta_y) > (cr + rd.y)) {
		result = false;
	} else {
		//Get intersection point of vector and nearest rect edge relative to the center of the rectangle
		float clamp_x = fminf(fmaxf(delta_x, -rd.x), rd.x);
		float clamp_y = fminf(fmaxf(delta_y, -rd.y), rd.y);
	
		// Get a vector pointing from the center of circle to center of rect
		delta_x = rp.x - cp.x;
		delta_y = rp.y - cp.y;

		// Get vector pointing from circle center to collision point
		clamp_x += delta_x;
		clamp_y += delta_y;

		float magnitude = hypotf(clamp_x, clamp_y);

		if (magnitude == 0.0f) magnitude = 1.0f; //Hack to avoid divide by zero when circle is inside rect
		if (magnitude < cr) {
			overlap->x = (clamp_x / magnitude) * (cr - magnitude); 
			overlap->y = (clamp_y / magnitude) * (cr - magnitude); 
			result = true;
		}
	}

	return result;
}

// Check for collision between circle and rectangle (left x, top y, width, height) and return overlap vector by reference
bool sc2d_check_circle_rect(sc2d_v2 cp, float cr, sc2d_v2 rp, sc2d_v2 rd, sc2d_v2* overlap) {
	bool result = false;

	rd.x /= 2.0f;
	rd.y /= 2.0f;

	rp.x += rd.x;
	rp.y += rd.y;

	result = sc2d_check_circle_centered_rect(cp, cr, rp, rd, overlap);

	return result;
}

// Project all points in polygon to 2D vector axis (dot product)
static inline void project_poly2d_to_axis(sc2d_v2 axis, sc2d_v2* poly_verts, int poly_vert_count, float* min, float* max) {
	*min=0; *max=0;
	
	for (int i = 0; i < poly_vert_count; i++) {
		float dot = (axis.x * poly_verts[i].x) + (axis.y * poly_verts[i].y); // dot product
		*min = fminf(*min, dot);
		*max = fmaxf(*max, dot);
	}
}

// Get vector from start index to next vertex in polygon
static inline void get_poly2d_edge(sc2d_v2* poly_verts, int vert_count, int start_index, sc2d_v2* edge) {
	int end_index = (start_index + 1) % vert_count; // wrap to first vertex

	edge->x = poly_verts[end_index].x - poly_verts[start_index].x;
	edge->y = poly_verts[end_index].y - poly_verts[start_index].y;
}

// Get clockwise or counterclockwise normal of 2D vector
static void v2_normal(sc2d_v2* v, bool clockwise) {
	float temp;

	temp = v->x;
	v->x  = v->y  * (float)(1 - (2 * (int)(!clockwise) ));
	v->y  = temp * (float)(1 - (2 * (int)(clockwise) ));
}

// Normalize 2d vector
static void v2_normalize(sc2d_v2* v) {
	float magnitude;

	magnitude = hypotf(v->x, v->y);
	v->x /= magnitude;
	v->y /= magnitude;
}

// Check for collision between two convex polygons and return shortest axis overlap by reference
bool sc2d_check_poly2d(	sc2d_v2 p1, sc2d_v2* p1_verts, int p1_count, 
						sc2d_v2 p2, sc2d_v2* p2_verts, int p2_count, 
						sc2d_v2* overlap) {

	bool result = true;
	float p1_min, p1_max, p2_min, p2_max;
	sc2d_v2 axis;

	float delta_x = p2.x - p1.x;
	float delta_y = p2.y - p1.y;
	float offset = 0;
	float min_distance = INFINITY;

	// First polygon
	for (int i = 0; i < p1_count; i++) {
		get_poly2d_edge(p1_verts, p1_count, i, &axis); // Get vector pointing from curent vertex to next vertex (edge)
		v2_normal(&axis, false); // Get the normal of the edge (vector perpendicular to the edge)
		v2_normalize(&axis);
		offset = (axis.x * delta_x) + (axis.y * delta_y); // project the the vector between polygon positions to the axis (dot product)

		project_poly2d_to_axis(axis, p1_verts, p1_count, &p1_min, &p1_max); // project ever y vertex in first polygon to current axis
		project_poly2d_to_axis(axis, p2_verts, p2_count, &p2_min, &p2_max); // project ever y vertex in scond polygon to current axis
		
		p1_min -= offset; // Add position offset to projection
		p1_max -= offset;
		
		if ( (p1_min > p2_max) || (p1_max < p2_min)) { // If the ranges do not overlap, polygons are not touching
			return false;
		}
		
		float distance = fminf(p1_max, p2_max) - fmaxf(p1_min, p2_min);
		if (distance < min_distance) { // Update minimum distance for overlap
			min_distance = distance;
			overlap->x = axis.x * (float)(1 - 2 * (int)(offset < 0) );
			overlap->y = axis.y * (float)(1 - 2 * (int)(offset < 0) );
		}
	}

	// Project all vertices to all axes of the second polygon
	for (int i = 0; i < p2_count; i++) {
		get_poly2d_edge(p2_verts, p2_count, i, &axis);
		v2_normal(&axis, false);
		v2_normalize(&axis);
		offset = (axis.x * delta_x) + (axis.y * delta_y);

		project_poly2d_to_axis(axis, p1_verts, p1_count, &p1_min, &p1_max);
		project_poly2d_to_axis(axis, p2_verts, p2_count, &p2_min, &p2_max);
		
		p1_min -= offset;
		p1_max -= offset;
		
		if ( (p1_min > p2_max) || (p1_max < p2_min)) {
			return false;
		}

		float distance = fminf(p1_max, p2_max) - fmaxf(p1_min, p2_min);
		if (distance < min_distance) {
			min_distance = distance;
			overlap->x = axis.x * (float)(1 - 2 * (int)(offset < 0) );
			overlap->y = axis.y * (float)(1 - 2 * (int)(offset < 0) );
		}
	}

	overlap->x *= min_distance;
	overlap->y *= min_distance;

	return result;
}

// Check for collision between point and convex polygon
bool sc2d_check_point_poly2d(sc2d_v2 p, sc2d_v2* poly_verts, int vert_count) {
	bool result = false;

	for (int i = 0, j = vert_count - 1; i < vert_count; i++) {
		if ( (poly_verts[i].y >= p.y) != (poly_verts[j].y >= p.y) &&
			 (p.x < (poly_verts[j].x - poly_verts[i].x) * (p.y - poly_verts[i].y) / (poly_verts[j].y - poly_verts[i].y) + poly_verts[i].x)
			) {
			
			result = !result;
		}

		j = i;
	}

	return result;
}

// Check for collision between point and line or line segment
bool sc2d_check_point_line(sc2d_v2 p, sc2d_v2 start, sc2d_v2 end, bool segment) {

	bool result = false;

	// Vector pointing from line start to line end
	float line_x = end.x - start.x;
	float line_y = end.y - start.y;
	float line_length = hypotf(line_y, line_x);

	// Vector point from line start to point
	float point_delta_x = p.x - start.x;
	float point_delta_y = p.y - start.y;
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
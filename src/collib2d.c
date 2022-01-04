#include <math.h>
#include <stdbool.h>

#define sign(num) (num > 0.0f) ? 1.0f : -1.0f

typedef struct range { float min, max; } range;

// Check for collion between a point and a circle and return penetration by reference
bool collib2d_check_point_circle(float px, float py, float cx, float cy, float cr, float* overlap_x, float* overlap_y) {
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
bool collib2d_check_point_rect(float px, float py, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
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
bool collib2d_check_circles(float x1, float y1, float r1, float x2, float y2, float r2, float* overlap_x, float* overlap_y) {
	bool result = false;

	float delta_x = x2 - x1;
	float delta_y = y2 - y1;
	float magnitude = hypotf(delta_x, delta_y);
	float overlap = (r1 + r2) - magnitude;

	if (result = overlap > 0) {
		*overlap_x = (delta_x / magnitude) * overlap;
		*overlap_y = (delta_y / magnitude) * overlap;
	}

	return result;
}

//Check for collision between to rectangles (left x, top y, width, height) and return overlap by reference
bool collib2d_check_rects(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2, float* overlap_x, float* overlap_y) {
	bool result = false;

	if (x1 < x2) 	*overlap_x = (x1 + w1) - x2;
	else 			*overlap_x = (x2 + w2) - x1;

	if (y1 < y2) 	*overlap_y = (y1 + h1) - y2;
	else			*overlap_y = (y2 + h2) - y1;
	
	if ( result = (*overlap_x > 0 && *overlap_y > 0) ) {
		if (x2 < x1) *overlap_x *= -1;
		if (y2 < y1) *overlap_y *= -1;

		*overlap_x *= (float)(int)(fabsf(*overlap_x) < fabsf(*overlap_y));
		*overlap_y *= (float)(int)(*overlap_x == 0);
	}

	return result;
}

// Check for collision of circle and centered rectangle (center x, centery, width, height) and return overlap by reference
bool collib2d_check_circle_centered_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
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

		// TO-DO: Handle containment
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
bool collib2d_check_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	float rect_center_width = rw/2.0f;
	float rect_center_height = rh/2.0f;

	float rect_center_x = rx + rect_center_width;
	float rect_center_y = ry + rect_center_height;

	result = collib2d_check_circle_centered_rect(cx, cy, cr, rect_center_x, rect_center_y, rect_center_width, rect_center_height, overlap_x, overlap_y);

	return result;
}

// Project all points in polygon to 2D vector axis (dot product)
static range project_poly2d_to_axis(float axis_x, float axis_y, float* poly_verts, int poly_vert_count) {
	range result = {0};

	for (int i = 0; i < poly_vert_count; i += 2) {
		float* vert_x = (poly_verts + i);
		float* vert_y = vert_x + 1;
		float dot = (axis_x * *vert_x) + (axis_y * *vert_y); // dot product
		result.min = fminf(result.min, dot);
		result.max = fmaxf(result.max, dot);
	}

	return result;
}

// Get vector from start index to next vertex in polygon
static void get_poly2d_edge(float* poly_verts, int vert_count, int startIndex, float* edge_x, float* edge_y) {
	float start_x, start_y;
	float end_x, end_y;

	start_x = *(poly_verts + startIndex++);
	start_y = *(poly_verts + startIndex);

	startIndex = (startIndex + 1) % vert_count; // wrap to first vertex
	
	end_x 	= *(poly_verts + startIndex++);
	end_y 	= *(poly_verts + startIndex);
	
	*edge_x = end_x - start_x;
	*edge_y = end_y - start_y;
}

// Get clockwise or counterclockwise normal of 2D vector
static void v2_normal(float* vx, float* vy, bool clockwise) {
	float temp;

	temp = *vx;
	*vx  = *vy  * (float)(1 - (2 * (int)(!clockwise) ));
	*vy  = temp * (float)(1 - (2 * (int)(clockwise) ));
}

// Normalize 2d vector
static void v2_normalize(float* vx, float* vy) {
	float magnitude;

	magnitude = hypotf(*vx, *vy);
	*vx /= magnitude;
	*vy /= magnitude;
}

// Check for collision between two convex polygons and return shortest axis overlap by reference
bool collib2d_check_poly2d(	float p1_pos_x, float p1_pos_y, float* p1_verts, int p1_vert_count, 
							float p2_pos_x, float p2_pos_y, float* p2_verts, int p2_vert_count, 
							float* overlap_x, float* overlap_y) {

	// p1_verts & p2_verts: float array of x/y pairs (vector2)
	// p1_vert_Count & p2_vert_count: total number of float values (vec2 * 2)

	bool result = true;
	range p1_projection, p2_projection;
	float axis_x, axis_y;

	float delta_x = p2_pos_x - p1_pos_x;
	float delta_y = p2_pos_y - p1_pos_y;
	float offset = 0;
	float min_distance = INFINITY;

	// First polygon
	for (int i = 0; i < p1_vert_count; i += 2) {
		get_poly2d_edge(p1_verts, p1_vert_count, i, &axis_x, &axis_y); // Get vector pointing from curent vertex to next vertex (edge)
		v2_normal(&axis_x, &axis_y, false); // Get the normal of the edge (vector perpendicular to the edge)
		v2_normalize(&axis_x, &axis_y);
		offset = (axis_x * delta_x) + (axis_y * delta_y); // project the the vector between polygon positions to the axis (dot product)

		p1_projection = project_poly2d_to_axis(axis_x, axis_y, p1_verts, p1_vert_count); // project every vertex in first polygon to current axis
		p2_projection = project_poly2d_to_axis(axis_x, axis_y, p2_verts, p2_vert_count); // project every vertex in scond polygon to current axis
		
		p1_projection.min -= offset; // Add position offset to projection
		p1_projection.max -= offset;
		
		if ( (p1_projection.min > p2_projection.max) || (p1_projection.max < p2_projection.min)) { // If the ranges do not overlap, polygons are not touching
			return false;
		}
		
		float distance = fminf(p1_projection.max, p2_projection.max) - fmaxf(p1_projection.min, p2_projection.min);
		if (distance < min_distance) { // Update minimum distance for overlap
			min_distance = distance;
			*overlap_x = axis_x * (float)(1 - 2 * (int)(offset < 0) );
			*overlap_y = axis_y * (float)(1 - 2 * (int)(offset < 0) );
		}
	}

	// Project all vertices to all axes of the second polygon
	for (int i = 0; i < p2_vert_count; i += 2) {
		get_poly2d_edge(p2_verts, p2_vert_count, i, &axis_x, &axis_y);
		v2_normal(&axis_x, &axis_y, false);
		v2_normalize(&axis_x, &axis_y);
		offset = (axis_x * delta_x) + (axis_y * delta_y);

		p1_projection = project_poly2d_to_axis(axis_x, axis_y, p1_verts, p1_vert_count);
		p2_projection = project_poly2d_to_axis(axis_x, axis_y, p2_verts, p2_vert_count);
		
		p1_projection.min -= offset;
		p1_projection.max -= offset;
		
		if ( (p1_projection.min > p2_projection.max) || (p1_projection.max < p2_projection.min)) {
			return false;
		}

		float distance = fminf(p1_projection.max, p2_projection.max) - fmaxf(p1_projection.min, p2_projection.min);
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
bool collib2d_check_point_poly2d(float point_x, float point_y, float* poly_verts, int vert_count) {
	bool result = false;

	for (int i = 0, j = vert_count - 2; i < vert_count; i += 2) {
		if ( (poly_verts[i + 1] >= point_y) != (poly_verts[j + 1] >= point_y) &&
			 (point_x < (poly_verts[j] - poly_verts[i]) * (point_y - poly_verts[i + 1]) / (poly_verts[j + 1] - poly_verts[i + 1]) + poly_verts[i])
			) {
			
			result = !result;
		}

		j = i;
	}

	return result;
}

// Check for collision between point and line or line segment
bool collib2d_check_point_line_intersect(float point_x, float point_y,
										 float line_start_x, float line_start_y, float line_end_x, float line_end_y,
										 bool segment) {

	bool result = false;

	// Vector pointing from line start to line end
	float line_x = line_end_x - line_start_x;
	float line_y = line_end_y - line_start_y;
	float line_length = hypotf(line_y, line_x);

	// Vector point from line start to point
	float point_delta_x = point_x - line_start_x;
	float point_delta_y = point_y - line_start_y;
	float distance_to_point = hypotf(point_delta_x, point_delta_y);

	// If the angles of both vectors are equal,
	// then the point is on the ray starting at line_start
	if (atan2(line_y, line_x) == atan2(point_delta_y, point_delta_x)) {
		// if the distance to the point is less than or equal to line length,
		// then the point is on the segment line_start -> line_end
		if (segment && (distance_to_point <= line_length) ) {
			result = true;
		// Else line_start->line_end describes an (infinite) line or a ray
		} else {
			result = true;
		}
	// If the angle of the line pointing in the opposite direction is equal
	// angle from line_start to the point,
	// then the point is still on the (infinite) line defined by line_start and line_end
	// but not the segment or ray defined by line_start and line_end
	} else if (!segment && (atan2(-line_x, -line_y) == atan2(point_delta_y, point_delta_x)) ) {
		result = true;
	}

	return result;
}
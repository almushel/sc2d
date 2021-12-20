#include <math.h>
#include <stdbool.h>

#define sign(num) (num > 0.0f) ? 1.0f : -1.0f

typedef struct range { float min, max; } range;

bool collib2d_check_point_circle(float px, float py, float cx, float cy, float cr, float* overlap_x, float* overlap_y) {
	bool result = false;

	float delta_x = cx - px;
	float delta_y = cy - py;

	float delta_m = hypot(delta_x, delta_y);
	float delta_r =  cr - delta_m;

	if (result = delta_r > 0) {
		*overlap_x = (delta_x / delta_m) * delta_r;
		*overlap_y = (delta_y / delta_m) * delta_r;
	}

	return result;
}

bool collib2d_check_point_rect(float px, float py, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	float rect_center_width = rw/2.0f;
	float rect_center_height = rh/2.0f;

	float rect_center_x = rx + rect_center_width;
	float rect_center_y = ry + rect_center_height;

	float delta_x = rect_center_x - px;
	float delta_y = rect_center_y - py;

	if (result = (fabs(delta_x) < rect_center_width && fabs(delta_y) < rect_center_height) ) {
		
		*overlap_x = (rect_center_width - fabs(delta_x));
		*overlap_y = (rect_center_height - fabs(delta_y));

		*overlap_x *= (float)(int)(*overlap_x < *overlap_y);
		*overlap_y *= (float)(int)(*overlap_x == 0);

		if (delta_y < 0) *overlap_y *= -1;
	}

	return result;
}

bool collib2d_check_circles(float x1, float y1, float r1, float x2, float y2, float r2, float* overlap_x, float* overlap_y) {
	bool result = false;

	float delta_x = x2 - x1;
	float delta_y = y2 - y1;
	float magnitude = hypot(delta_x, delta_y);
	float overlap = (r1 + r2) - magnitude;

	if (result = overlap > 0) {
		*overlap_x = (delta_x / magnitude) * overlap;
		*overlap_y = (delta_y / magnitude) * overlap;
	}

	return result;
}

bool collib2d_check_rects(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2, float* overlap_x, float* overlap_y) {
	bool result = false;

	if (x1 < x2) 	*overlap_x = (x1 + w1) - x2;
	else 			*overlap_x = (x2 + w2) - x1;

	if (y1 < y2) 	*overlap_y = (y1 + h1) - y2;
	else			*overlap_y = (y2 + h2) - y1;
	
	if ( result = (*overlap_x > 0 && *overlap_y > 0) ) {
		if (x2 < x1) *overlap_x *= -1;
		if (y2 < y1) *overlap_y *= -1;

		*overlap_x *= (float)(int)(fabs(*overlap_x) < fabs(*overlap_y));
		*overlap_y *= (float)(int)(*overlap_x == 0);
	}

	return result;
}

bool collib2d_check_circle_centered_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	// Get a vector pointing from center of rect to center of circle
	float delta_x = cx - rx;
	float delta_y = cy - ry;

	if ( fabs(delta_x) > (cr + rw) || fabs(delta_y) > (cr + rh)) {
		result = false;
	} else {
		//Get intersection point of vector and nearest rect edge relative to the center of the rectangle
		float clamp_x = fmin(fmax(delta_x, -rw), rw);
		float clamp_y = fmin(fmax(delta_y, -rh), rh);
	
		// Get a vector point from the center of circle to center of rect
		delta_x = rx - cx;
		delta_y = ry - cy;

		// Get vector point from circle center to collision point?
		clamp_x += delta_x;
		clamp_y += delta_y;

		float magnitude = hypot(clamp_x, clamp_y);

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

bool collib2d_check_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	float rect_center_width = rw/2.0f;
	float rect_center_height = rh/2.0f;

	float rect_center_x = rx + rect_center_width;
	float rect_center_y = ry + rect_center_height;

	result = collib2d_check_circle_centered_rect(cx, cy, cr, rect_center_x, rect_center_y, rect_center_width, rect_center_height, overlap_x, overlap_y);

	return result;
}

static range project_poly2d_to_axis(float axis_x, float axis_y, float* poly_verts, int poly_vert_count) {
	range result = {0};

	for (int i = 0; i < poly_vert_count; i += 2) {
		float* vert_x = (poly_verts + i);
		float* vert_y = vert_x + 1;
		float dot = (axis_x * *vert_x) + (axis_y * *vert_y); // dot product
		result.min = fmin(result.min, dot);
		result.max = fmax(result.max, dot);
	}

	return result;
}

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

static void v2_normal(float* vx, float* vy, bool clockwise) {
	float temp;

	temp = *vx;
	*vx  = *vy  * (float)(1 - (2 * (int)(!clockwise) ));
	*vy  = temp * (float)(1 - (2 * (int)(clockwise) ));
}

static void v2_normalize(float* vx, float* vy) {
	float magnitude;

	magnitude = hypot(*vx, *vy);
	*vx /= magnitude;
	*vy /= magnitude;
}

bool collib2d_check_poly2d(	float p1_pos_x, float p1_pos_y, float* p1_verts, int p1_vert_count, 
							float p2_pos_x, float p2_pos_y, float* p2_verts, int p2_vert_count, 
							float* overlap_x, float* overlap_y) {

	// p1_verts & p2_verts: float array of x/y pairs (vector2)

	bool result = true;
	range p1Projection, p2Projection;
	float axis_x, axis_y;
	float* vert_x; 
	float* vert_y;

	float delta_x = p2_pos_x - p1_pos_x;
	float delta_y = p2_pos_y - p1_pos_y;
	float offset = 0;
	float minDistance = INFINITY;

	// First polygon
	for (int i = 0; i < p1_vert_count; i += 2) {
		vert_x = p1_verts + i;
		vert_y = vert_x + 1;

		get_poly2d_edge(p1_verts, p1_vert_count, i, &axis_x, &axis_y);
		v2_normal(&axis_x, &axis_y, true);
		v2_normalize(&axis_x, &axis_y);
		offset = (axis_x * delta_x), + (axis_y * delta_y); // dot product

		p1Projection = project_poly2d_to_axis(axis_x, axis_y, p1_verts, p1_vert_count);
		p1Projection.min += offset;
		p1Projection.max += offset;
		p2Projection = project_poly2d_to_axis(axis_x, axis_y, p2_verts, p2_vert_count);
		
		if ( (p1Projection.min > p2Projection.max) || (p1Projection.max < p2Projection.min)) {
			return false;
		}
		
		float distance = fmin(p1Projection.max, p2Projection.max) - fmax(p1Projection.min, p2Projection.min);
		if (distance < minDistance) {
			minDistance = distance;
//			*overlap = Vector2Scale(axis, offset > 0 ? 1 : -1);
			*overlap_x = axis_x * (float)(1 - 2 * (int)(offset < 0) );
			*overlap_y = axis_y * (float)(1 - 2 * (int)(offset < 0) );
		}
	}

	// Second Polygon
	for (int i = 0; i < p2_vert_count; i++) {
		get_poly2d_edge(p2_verts, p2_vert_count, i, &axis_x, &axis_y);
		v2_normal(&axis_x, &axis_y, true);
		v2_normalize(&axis_x, &axis_y);
		offset = (axis_x * delta_x) + (axis_y * delta_y);

		p1Projection = project_poly2d_to_axis(axis_x, axis_y, p1_verts, p1_vert_count);
		p1Projection.min += offset;
		p1Projection.max += offset;
		p2Projection = project_poly2d_to_axis(axis_x, axis_y, p2_verts, p2_vert_count);
		
		if ( (p1Projection.min > p2Projection.max) || (p1Projection.max < p2Projection.min)) {
			return false;
		}
		

		float distance = fmin(p1Projection.max, p2Projection.max) - fmax(p1Projection.min, p2Projection.min);
		if (distance < minDistance) {
			minDistance = distance;
			*overlap_x = axis_x * (float)(1 - 2 * (int)(offset < 0) );
			*overlap_y = axis_y * (float)(1 - 2 * (int)(offset < 0) );
		}
	}

	*overlap_x *= minDistance;
	*overlap_y *= minDistance;

	return result;
}
#include <math.h>
#include <stdbool.h>

static inline float sign(float num) { 
	float result = 1.0f - (2.0f * (float)(num < 0) );
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

bool collib2d_check_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	float rect_center_width = rw/2.0f;
	float rect_center_height = rh/2.0f;

	float rect_center_x = rx + rect_center_width;
	float rect_center_y = ry + rect_center_height;

	// Get a vector pointing from center of rect to center of circle
	float delta_x = cx - rect_center_x;
	float delta_y = cy - rect_center_y;

	if ( fabs(delta_x) > (cr + rect_center_width) || fabs(delta_y) > (cr + rect_center_height)) {
		result = false;
	} else {
		//Get intersection point of vector and nearest rect edge
		float clamp_x = fmin(fmax(delta_x, -rect_center_width), rect_center_width);
		float clamp_y = fmin(fmax(delta_y, -rect_center_height), rect_center_height);
	
		// Get a vector point from the center of circle to center of rect
		delta_x = rect_center_x - cx;
		delta_y = rect_center_y - cy;

		// Get vector point from circle center to collision point?
		clamp_x += delta_x;
		clamp_y += delta_y;

		float magnitude = hypot(clamp_x, clamp_y);

		if (magnitude <= cr) {
			*overlap_x = (clamp_x / magnitude) * (cr - magnitude); 
			*overlap_y = (clamp_y / magnitude) * (cr - magnitude); 
			result = true;
		}
	}

	return result;
}
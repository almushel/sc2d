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
	
	result = (*overlap_x > 0 && *overlap_y > 0);

	if (x2 < x1) *overlap_x *= -1;
	if (y2 < y1) *overlap_y *= -1;

	*overlap_x *= (float)(int)(fabs(*overlap_x) < fabs(*overlap_y));
	*overlap_y *= (float)(int)(*overlap_x == 0);

	return result;
}

bool collib2d_check_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y) {
	bool result = false;

	float rect_center_x = rx + rw/2.0f;
	float rect_center_y = ry + rh/2.0f;

	float delta_x = rect_center_x - cx;
	float delta_y = rect_center_y - cy;

	if ( fabs(delta_x) < (cr + rw/2) && fabs(delta_y) < (cr + rh/2)) {
		result = true;

		*overlap_x = fabs(delta_x) - (cr + rw/2);
		*overlap_x *= -sign(delta_x);
		
		*overlap_y = fabs(delta_y) - (cr + rh/2);
		*overlap_y *= -sign(delta_y);
	}

	*overlap_x *= (float)(int)(fabs(*overlap_x) < fabs(*overlap_y));
	*overlap_y *= (float)(int)(*overlap_x == 0);

	return result;
}
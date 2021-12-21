
bool collib2d_check_point_circle(float px, float py, float cx, float cy, float cr, float* overlap_x, float* overlap_y);
bool collib2d_check_point_rect(float px, float py, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y);
bool collib2d_check_circles(float x1, float y1, float r1, float x2, float y2, float r2, float* overlap_x, float* overlap_y);
bool collib2d_check_rects(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2, float* overlap_x, float* overlap_y);
bool collib2d_check_circle_centered_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y);
bool collib2d_check_circle_rect(float cx, float cy, float cr, float rx, float ry, float rw, float rh, float* overlap_x, float* overlap_y);

bool collib2d_check_poly2d(	float p1_pos_x, float p1_pos_y, float* p1_verts, int p1_vert_count, 
							float p2_pos_x, float p2_pos_y, float* p2_verts, int p2_vert_count, 
							float* overlap_x, float* overlap_y);
bool collib2d_check_point_poly2d(float point_x, float point_y, float* poly_verts, int vert_count);
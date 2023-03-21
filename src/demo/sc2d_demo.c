// NOTE: Currently uses C99 features. May not be fully compatible with C++

#include "raylib.h"
#include "raymath.h"

// NOTE: This demo uses Vector3 instead of Vector2 to demonstrate non-standard structs in the sc2d_check_polygon functions
// The z value is entirely unused here and assumed to be zero
// For polygons defined by order paired of x/y values, this typedef is unnecessary

#ifndef SIMPLE_COLLISION_2D_VECTOR2
typedef Vector3 sc2d_v2;
#define SIMPLE_COLLISION_2D_VECTOR2
#endif

#define SIMPLE_COLLISION_2D_IMPLEMENTATION 1
#include "sc2d.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define SCROLL_INCREMENT 2.0f

#define arrayLength(array) sizeof(array) / sizeof(array[0])

#define POLYGON_SCALE 20.0f
#define MAX_POLY_SIDES 12

typedef struct Polygon {
	Vector3 vertices[MAX_POLY_SIDES];
	int vert_count;
} Polygon;

typedef struct Object {
	enum ObjectType {OBJECT_TYPE_CIRCLE, OBJECT_TYPE_RECTANGLE, OBJECT_TYPE_POLYGON} type;
	Vector3 position;
	float scale;
	float rotation;
	Polygon shape;
	bool hit;
} Object;

static Vector3 random_screen_coords() {
	Vector3 result = {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT), 0};
	return result;
}

// Polygon functions

void log_poly_verts(Polygon p) {
	for (int i = 0; i < p.vert_count; i++) {
		TraceLog(LOG_INFO, "Vertex %d X: %f, Y: %f\n", i, p.vertices[i].x, p.vertices[i].y);
	}
}

Polygon generate_polygon(int sides) {
	Polygon result = {.vert_count = sides};
	float angle = 0;
	float offset = 2 * PI / sides;
	
	for (int i = 0; i < result.vert_count; i++) {
		result.vertices[i] = (Vector3){cos(angle), sin(angle), 0};
		angle += offset;
	}

	return result;
}

Polygon scale_polygon(Polygon p, float scale) {
	Polygon result = {.vert_count = p.vert_count};
	for (int i = 0; i < result.vert_count; i++) {
		result.vertices[i] = Vector3Scale(p.vertices[i], scale);
	}
	return result;
}

Polygon translate_polygon(Polygon p, Vector3 translation) {
	Polygon result = {.vert_count = p.vert_count};
	for (int i = 0; i < result.vert_count; i++) {
		result.vertices[i] = Vector3Add(p.vertices[i], translation);
	}

	return result;
}

Polygon rotate_polygon(Polygon p, float degrees) {
	Polygon result;

	if (degrees) {
		float angle = degrees*DEG2RAD;
		result = (Polygon){.vert_count = p.vert_count};
		for (int i = 0; i < result.vert_count; i++) {
			Vector2 v = Vector2Rotate((Vector2){p.vertices[i].x, p.vertices[i].y}, angle);
			result.vertices[i] = (Vector3){v.x, v.y, 0};
		}
	} else result = p;

	return result;
}

Vector3 get_polygon_edge(Polygon p, int startIndex) {
	Vector3 result = 
		Vector3Subtract(p.vertices[(startIndex + 1) % p.vert_count], p.vertices[startIndex]);
	return result;
}

Rectangle poly_to_rect(Vector3 position, Polygon p) {
	Rectangle result = {0};

	if (p.vert_count > 4) TraceLog(LOG_WARNING, "Converted poly with more than 4 vertices to rect.");

	result.width = fabs(p.vertices[2].x - p.vertices[0].x);				
	result.height = fabs(p.vertices[2].y - p.vertices[0].y);

	result.x = position.x - result.width/2.0f;
	result.y = position.y - result.height/2.0f;

	return result;
}

void draw_polygon(Vector3 v, Polygon p, Color c) {
	if (p.vert_count < 2) {
		TraceLog(LOG_WARNING, "Polygon has less than 2 verts and was not drawn");
		return;
	}
	p = translate_polygon(p, v);

	//DrawLineStrip(p.vertices, p.vert_count, c);
	for (int i = 0; i < p.vert_count; i ++) {
		Vector2 v1 = {p.vertices[i].x, p.vertices[i].y};
		Vector2 v2 = {p.vertices[(i+1) % p.vert_count].x, p.vertices[(i+1) % p.vert_count].y};
		DrawLineV(v1, v2, c); //Draw one more line to close the shape
	}
	//DrawLineV(p.vertices[p.vert_count - 1], p.vertices[0], c); //Draw one more line to close the shape
}

// Object functions

static void draw_object_label(Vector3 pos, int type) {
	switch(type) {
		case OBJECT_TYPE_CIRCLE:
			DrawText("Circle", pos.x, pos.y, 16, WHITE);
			break;
		case OBJECT_TYPE_RECTANGLE:
			DrawText("Rectangle", pos.x, pos.y, 16, WHITE);
			break;
		case OBJECT_TYPE_POLYGON:
			DrawText("Polygon", pos.x, pos.y, 16, WHITE);
			break;
		default:
			break;
	}
}

bool check_object_collision(Object obj1, Object obj2, Vector2* overlap) {
	bool result = false;
	Polygon p1, p2;

	p1 = scale_polygon(obj1.shape, obj1.scale);
	p1 = rotate_polygon(p1, obj1.rotation);

	p2 = scale_polygon(obj2.shape, obj2.scale);
	p2 = rotate_polygon(p2, obj2.rotation);

	switch (obj1.type) {
		// obj1 is a circle
		case OBJECT_TYPE_CIRCLE: {
			switch (obj2.type) {
				case OBJECT_TYPE_CIRCLE:
					result = sc2d_check_circles(obj1.position.x, obj1.position.y, obj1.scale, 
												obj2.position.x, obj2.position.y, obj2.scale, 
												&overlap->x, &overlap->y);
					break;
				case OBJECT_TYPE_RECTANGLE: {
					Rectangle rect = poly_to_rect(obj2.position, p2);
					result = sc2d_check_circle_rect(obj1.position.x, obj1.position.y, obj1.scale, 
													rect.x, rect.y, rect.width, rect.height,
													&overlap->x, &overlap->y);
				}

					break;
				case OBJECT_TYPE_POLYGON: {
					p1 = generate_polygon(MAX_POLY_SIDES); // circle
					p1 = scale_polygon(p1, obj1.scale);

					result = sc2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vert_count, 
												obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vert_count,
												&overlap->x, &overlap->y);
				} break;
				
				default:
					break;
			}
		} break;
		
		// obj1 is a rectangle
		case OBJECT_TYPE_RECTANGLE: {
			switch (obj2.type) {
				case OBJECT_TYPE_CIRCLE: {
					Rectangle rect = poly_to_rect(obj1.position, p1);

					result = sc2d_check_circle_rect(obj2.position.x, obj2.position.y, obj2.scale, 
													rect.x, rect.y, rect.width, rect.height,
													&overlap->x, &overlap->y);
					*overlap = Vector2Scale(*overlap, -1.0f); // Flip the overlap, because the circle is the second object
				} break;
				
				case OBJECT_TYPE_RECTANGLE:{
					Rectangle rect1 = poly_to_rect(obj1.position, p1);
					Rectangle rect2 = poly_to_rect(obj2.position, p2);

					result = sc2d_check_rects(	rect1.x, rect1.y, rect1.width, rect1.height, 
												rect2.x, rect2.y, rect2.width, rect2.height,
												&overlap->x, &overlap->y);
				} break;
				case OBJECT_TYPE_POLYGON: {
					result = sc2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vert_count, 
												obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vert_count,
												&overlap->x, &overlap->y);
				} break;
				
				default:
					break;
			}
		} break;
		
		//obj1 is a polygon
		case OBJECT_TYPE_POLYGON: {
			switch (obj2.type) {
				case OBJECT_TYPE_CIRCLE: {
					p2 = generate_polygon(MAX_POLY_SIDES); // circle
					p2 = scale_polygon(p2, obj2.scale);

					result = sc2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vert_count, 
												obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vert_count,
												&overlap->x, &overlap->y);
				} break;

				case OBJECT_TYPE_RECTANGLE: {
					result = sc2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vert_count, 
												obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vert_count,
												&overlap->x, &overlap->y);
				} break;
				
				case OBJECT_TYPE_POLYGON: {				
					result = sc2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vert_count, 
												obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vert_count,
												&overlap->x, &overlap->y);
				} break;

				default: {} break;
			}
		} break;
		
		default:
			break;
	}

	return result;
}

int main(void) {
	Polygon triangle = generate_polygon(3);
	Polygon square = generate_polygon(4);
	square = rotate_polygon(square, 45);
	Polygon rect = square;
	Polygon pentagon = generate_polygon(5);
	pentagon = rotate_polygon(pentagon, -90);
	Polygon hexagon = generate_polygon(6);
	Polygon octagon = generate_polygon(8);

	Object objects[] = {
		{ .type = OBJECT_TYPE_CIRCLE, .scale = POLYGON_SCALE, },
		
		{ .type = OBJECT_TYPE_CIRCLE, .scale = POLYGON_SCALE * 1.5, },
		
		{ .type = OBJECT_TYPE_POLYGON, .shape = triangle, .scale = POLYGON_SCALE, },

		{ .type = OBJECT_TYPE_RECTANGLE, .shape = square, .scale = POLYGON_SCALE, },

		{ .type = OBJECT_TYPE_RECTANGLE, .shape = rect, .scale = POLYGON_SCALE * 4, },

		{ .type = OBJECT_TYPE_POLYGON, .shape = pentagon, .scale = POLYGON_SCALE, },

		{ .type = OBJECT_TYPE_POLYGON, .shape = hexagon, .scale = POLYGON_SCALE, },

		{ .type = OBJECT_TYPE_POLYGON, .shape = octagon, .scale = POLYGON_SCALE, }
	};

	for (int i = 0; i < arrayLength(objects); i++) {
		objects[i].position = random_screen_coords();
	}

	int shape_grabbed = -1;
	
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple Collision 2D");
	
	while(!WindowShouldClose()) {

		Vector2 mouse_position = GetMousePosition();
		Vector2 overlap = {0};

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			for (int i = 0; i < arrayLength(objects); i++) {
				bool hit = false;
				if (objects[i].type == OBJECT_TYPE_CIRCLE) {
					hit = sc2d_check_point_circle(	mouse_position.x, mouse_position.y, 
													objects[i].position.x, objects[i].position.y, objects[i].scale, &overlap.x, &overlap.y);

				} else {
					Polygon p = rotate_polygon(objects[i].shape, objects[i].rotation);
					p = scale_polygon(p, objects[i].scale);
					p = translate_polygon(p, objects[i].position);
					hit = sc2d_check_point_poly2d(mouse_position.x, mouse_position.y, (float*)p.vertices, p.vert_count);
				}

				if (hit) {
					shape_grabbed = i;
					break;
				}
			}
		}

		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || !IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
			shape_grabbed = -1;
		}
		
		if (shape_grabbed >= 0) { // Polygons
			objects[shape_grabbed].position = (Vector3){mouse_position.x, mouse_position.y, 0};
			float md = GetMouseWheelMove();
			objects[shape_grabbed].scale += md / fabs(md + (md == 0)) * SCROLL_INCREMENT;
		
		}
		
		BeginDrawing();
			ClearBackground((Color){0,0,0,255});
			Polygon p = {0};
			for (int i = 0; i < arrayLength(objects); i++) {
				Color color = (shape_grabbed == i) ? YELLOW : WHITE;

				for (int e = i+1; e < arrayLength(objects); e++) {
					//if (e == i) continue;
					overlap = (Vector2){0};
					
					bool hit = check_object_collision(objects[i], objects[e], &overlap);

					if (hit) {
						color = RED;
						if (i == shape_grabbed) {
							Vector2 v = Vector2Add((Vector2){objects[e].position.x, objects[e].position.y}, overlap);
							objects[e].position = (Vector3){v.x, v.y, 0};
						} else {
							Vector2 v = Vector2Subtract((Vector2){objects[i].position.x, objects[i].position.y}, overlap);
							objects[i].position = (Vector3){v.x, v.y, 0};
						}
					}
				}

				if (objects[i].type == OBJECT_TYPE_CIRCLE) {
					DrawCircleLines(objects[i].position.x, objects[i].position.y, objects[i].scale, color);
				} else {
					p = rotate_polygon(objects[i].shape, objects[i].rotation);
					p = scale_polygon(p, objects[i].scale);
					draw_polygon(objects[i].position, p, color);
				}
				draw_object_label(objects[i].position, objects[i].type);
			}

			DrawText("Click to grab and drag polygons.", 32, 32, 24, WHITE);
			DrawText("Scroll mouse wheel to resize currently grabbed polygon.", 32, 64, 24, WHITE);

		EndDrawing();
	}

	return 0;
}
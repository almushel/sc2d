// NOTE: Currently uses C99 features. May not be fully compatible with C++

#include "raylib.h"
#include "raymath.h"

#define SIMPLE_COLLISION_2D_TYPES
typedef Vector2 sc2d_v2;

#define SIMPLE_COLLISION_2D_IMPLEMENTATION 1
#include "sc2d.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define SCROLL_INCREMENT 2.0f

#define arrayLength(array) sizeof(array) / sizeof(array[0])

#define POLYGON_SCALE 20.0f
#define MAX_POLY_SIDES 12

typedef struct Polygon {
	Vector2 vertices[MAX_POLY_SIDES];
	int vertCount;
} Polygon;

typedef struct Object {
	enum ObjectType {OBJECT_TYPE_CIRCLE, OBJECT_TYPE_RECTANGLE, OBJECT_TYPE_POLYGON} type;
	Vector2 position;
	float scale;
	float rotation;
	Polygon shape;
	bool hit;
} Object;

static Vector2 getRandomScreenCoords() {
	Vector2 result = {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT)};
	return result;
}

// Polygon functions

void logPolyVerts(Polygon p) {
	for (int i = 0; i < p.vertCount; i++) {
		TraceLog(LOG_INFO, "Vertex %d X: %f, Y: %f\n", i, p.vertices[i].x, p.vertices[i].y);
	}
}

Polygon generatePolygon(int sides) {
	Polygon result = {.vertCount = sides};
	float angle = 0;
	float offset = 2 * PI / sides;
	
	for (int i = 0; i < result.vertCount; i++) {
		result.vertices[i] = (Vector2){cos(angle), sin(angle) };
		angle += offset;
	}

	return result;
}

Polygon scalePolygon(Polygon p, float scale) {
	Polygon result = {.vertCount = p.vertCount};
	for (int i = 0; i < result.vertCount; i++) {
		result.vertices[i] = Vector2Scale(p.vertices[i], scale);
	}
	return result;
}

Polygon translatePolygon(Polygon p, Vector2 translation) {
	Polygon result = {.vertCount = p.vertCount};
	for (int i = 0; i < result.vertCount; i++) {
		result.vertices[i] = Vector2Add(p.vertices[i], translation);
	}

	return result;
}

Polygon rotatePolygon(Polygon p, float degrees) {
	Polygon result;

	if (degrees) {
		float angle = degrees*DEG2RAD;
		result = (Polygon){.vertCount = p.vertCount};
		for (int i = 0; i < result.vertCount; i++) {
			result.vertices[i] = Vector2Rotate(p.vertices[i], angle);
		}
	} else result = p;

	return result;
}

Vector2 getPolygonEdge(Polygon p, int startIndex) {
	Vector2 result = 
		Vector2Subtract(p.vertices[(startIndex + 1) % p.vertCount], p.vertices[startIndex]);
	return result;
}

Rectangle polyToRect(Vector2 position, Polygon p) {
	Rectangle result = {0};

	if (p.vertCount > 4) TraceLog(LOG_WARNING, "Converted poly with more than 4 vertices to rect.");

	result.width = fabs(p.vertices[2].x - p.vertices[0].x);				
	result.height = fabs(p.vertices[2].y - p.vertices[0].y);

	result.x = position.x - result.width/2.0f;
	result.y = position.y - result.height/2.0f;

	return result;
}

void drawPolygon(Vector2 v, Polygon p, Color c) {
	if (p.vertCount < 2) {
		TraceLog(LOG_WARNING, "Polygon has less than 2 verts and was not drawn");
		return;
	}
	p = translatePolygon(p, v);

	DrawLineStrip(p.vertices, p.vertCount, c);
	DrawLineV(p.vertices[p.vertCount - 1], p.vertices[0], c); //Draw one more line to close the shape
}

// Object functions

static void drawObjectLabel(Vector2 pos, int type) {
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

bool checkObjectCollision(Object obj1, Object obj2, Vector2* overlap) {
	bool result = false;
	Polygon p1, p2;

	p1 = scalePolygon(obj1.shape, obj1.scale);
	p1 = rotatePolygon(p1, obj1.rotation);

	p2 = scalePolygon(obj2.shape, obj2.scale);
	p2 = rotatePolygon(p2, obj2.rotation);

	switch (obj1.type) {
		// obj1 is a circle
		case OBJECT_TYPE_CIRCLE: {
			switch (obj2.type) {
				case OBJECT_TYPE_CIRCLE:
					result = sc2d_check_circles(obj1.position, obj1.scale, 
												obj2.position, obj2.scale, 
												overlap);
					break;
				case OBJECT_TYPE_RECTANGLE: {
					Rectangle rect = polyToRect(obj2.position, p2);
					result = sc2d_check_circle_rect(obj1.position, obj1.scale, 
														(Vector2){rect.x, rect.y}, (Vector2){rect.width, rect.height},
														overlap);
				}

					break;
				case OBJECT_TYPE_POLYGON: {
					p1 = generatePolygon(MAX_POLY_SIDES); // circle
					p1 = scalePolygon(p1, obj1.scale);

					result = sc2d_check_poly2d(	obj1.position, p1.vertices, p1.vertCount, 
													obj2.position, p2.vertices, p2.vertCount,
													overlap);
				} break;
				
				default:
					break;
			}
		} break;
		
		// obj1 is a rectangle
		case OBJECT_TYPE_RECTANGLE: {
			switch (obj2.type) {
				case OBJECT_TYPE_CIRCLE: {
					Rectangle rect = polyToRect(obj1.position, p1);

					result = sc2d_check_circle_rect(obj2.position, obj2.scale, 
														(Vector2){rect.x, rect.y}, (Vector2){rect.width, rect.height},
														overlap);
					*overlap = Vector2Scale(*overlap, -1.0f); // Flip the overlap, because the circle is the second object
				} break;
				
				case OBJECT_TYPE_RECTANGLE:{
					Rectangle rect1 = polyToRect(obj1.position, p1);
					Rectangle rect2 = polyToRect(obj2.position, p2);

					result = sc2d_check_rects(	(Vector2){rect1.x, rect1.y}, (Vector2){rect1.width, rect1.height}, 
												(Vector2){rect2.x, rect2.y}, (Vector2){rect2.width, rect2.height},
												overlap);
				} break;
				case OBJECT_TYPE_POLYGON: {
					result = sc2d_check_poly2d(	obj1.position, p1.vertices, p1.vertCount, 
													obj2.position, p2.vertices, p2.vertCount,
													overlap);
				} break;
				
				default:
					break;
			}
		} break;
		
		//obj1 is a polygon
		case OBJECT_TYPE_POLYGON: {
			switch (obj2.type) {
				case OBJECT_TYPE_CIRCLE: {
					p2 = generatePolygon(MAX_POLY_SIDES); // circle
					p2 = scalePolygon(p2, obj2.scale);

					result = sc2d_check_poly2d(	obj1.position, p1.vertices, p1.vertCount, 
													obj2.position, p2.vertices, p2.vertCount,
													overlap);
				}
					break;
				case OBJECT_TYPE_RECTANGLE: {
					result = sc2d_check_poly2d(	obj1.position, p1.vertices, p1.vertCount, 
													obj2.position, p2.vertices, p2.vertCount,
													overlap);
				} break;
				
				case OBJECT_TYPE_POLYGON: {				
					result = sc2d_check_poly2d(	obj1.position, p1.vertices, p1.vertCount, 
													obj2.position, p2.vertices, p2.vertCount,
													overlap);
				} break;
				default:
					break;
			}
		} break;
		
		default:
			break;
	}

	return result;
}

int main(void) {
	Polygon triangle = generatePolygon(3);
	Polygon square = generatePolygon(4);
	square = rotatePolygon(square, 45);
	Polygon rect = square;
	Polygon pentagon = generatePolygon(5);
	pentagon = rotatePolygon(pentagon, -90);
	Polygon hexagon = generatePolygon(6);
	Polygon octagon = generatePolygon(8);

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
		objects[i].position = getRandomScreenCoords();
	}


	int shapeGrabbed = -1;
	
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple Collision 2D");
	
	while(!WindowShouldClose()) {

		Vector2 mousePosition = GetMousePosition();
		Vector2 overlap = {0};

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			for (int i = 0; i < arrayLength(objects); i++) {
				bool hit = false;
				if (objects[i].type == OBJECT_TYPE_CIRCLE) {
					hit = sc2d_check_point_circle(mousePosition, objects[i].position, objects[i].scale, &overlap);

				} else {
					Polygon p = rotatePolygon(objects[i].shape, objects[i].rotation);
					p = scalePolygon(p, objects[i].scale);
					p = translatePolygon(p, objects[i].position);
					hit = sc2d_check_point_poly2d(mousePosition, p.vertices, p.vertCount);
				}

				if (hit) {
					shapeGrabbed = i;
					break;
				}
			}
		}

		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || !IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
			shapeGrabbed = -1;
		}
		
		if (shapeGrabbed >= 0) { // Polygons
			objects[shapeGrabbed].position = mousePosition;
			float md = GetMouseWheelMove();
			objects[shapeGrabbed].scale += md / fabs(md + (md == 0)) * SCROLL_INCREMENT;
		
		}
		
		BeginDrawing();
			ClearBackground((Color){0,0,0,255});
			Polygon p = {0};
			for (int i = 0; i < arrayLength(objects); i++) {
				Color color = (shapeGrabbed == i) ? YELLOW : WHITE;

				for (int e = 0; e < arrayLength(objects); e++) {
					if (e == i) continue;
					overlap = (Vector2){0};
					
					bool hit = checkObjectCollision(objects[i], objects[e], &overlap);

					if (hit) {
						color = RED;
						if (i == shapeGrabbed) break;
						objects[i].position = Vector2Subtract(objects[i].position, overlap);
						// TraceLog(LOG_INFO, "overlap.x: %f, overlap.y: %f\n", overlap.x, overlap.y);
					}
				}

				if (objects[i].type == OBJECT_TYPE_CIRCLE) {
					DrawCircle(objects[i].position.x, objects[i].position.y, objects[i].scale, color);
				} else {
					p = rotatePolygon(objects[i].shape, objects[i].rotation);
					p = scalePolygon(p, objects[i].scale);
					drawPolygon(objects[i].position, p, color);
				}
				drawObjectLabel(objects[i].position, objects[i].type);
			}

			DrawText("Click to grab and drag polygons.", 32, 32, 24, WHITE);
			DrawText("Scroll mouse wheel to resize currently grabbed polygon.", 32, 64, 24, WHITE);

		EndDrawing();
	}

	return 0;
}
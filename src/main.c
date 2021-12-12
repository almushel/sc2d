#include "raylib.h"
#include "raymath.h"
#include "collib2d.h"
#include "polygon.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define SCROLL_INCREMENT 2.0f

#define POLYGON_SCALE 20.0f

#define arrayLength(array) sizeof(array) / sizeof(array[0])

typedef struct Object {
	enum ObjectType {circle, rectangle, polygon} type;
	Vector2 position;
	float scale;
	float rotation;
	Polygon shape;
	bool hit;
} Object;

float normalizeDegrees(float degrees);
bool checkObjectCollision(Object obj1, Object obj2, Vector2* overlap);

static Vector2 getRandomScreenCoords() {
	Vector2 result = {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT)};
	return result;
}

int main() {

	Polygon triangle = generatePolygon(3);
	Polygon square = { 	
		.vertices = { {0, 0}, {1, 0}, {1, 1}, {0, 1} },
		.vertCount = 4,
	};
	Polygon rect = { 	
		.vertices = { {0, 0}, {1, 0}, {1, 0.5f}, {0, 0.5f} },
		.vertCount = 4,
	};
	Polygon pentagon = generatePolygon(5);
	pentagon = rotatePolygon(pentagon, -90);
	Polygon hexagon = generatePolygon(6);
	Polygon octagon = generatePolygon(8);

	Object objects[] = {
		{
			.type = circle,
			.scale = POLYGON_SCALE,
		},
		
		{
			.type = circle,
			.scale = POLYGON_SCALE * 1.5,
		},
		
		{
			.type = polygon,
			.shape = triangle,
			.scale = POLYGON_SCALE,
		},

		{
			.type = rectangle,
			.shape = square,
			.scale = POLYGON_SCALE,
		},

		{
			.type = rectangle,
			.shape = rect,
			.scale = POLYGON_SCALE * 4,
		},

		{
			.type = polygon,
			.shape = pentagon,
			.scale = POLYGON_SCALE,
		},

		{
			.type = polygon,
			.shape = hexagon,
			.scale = POLYGON_SCALE,
		},

		{
			.type = polygon,
			.shape = octagon,
			.scale = POLYGON_SCALE,
		}
	};

	for (int i = 0; i < arrayLength(objects); i++) {
		GetRandomValue(0, SCREEN_WIDTH);
		objects[i].position = getRandomScreenCoords();
	}


	int shapeGrabbed = -1;
	
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Separating Axis Theorem");
	
	while(!WindowShouldClose()) {

		Vector2 mousePosition = GetMousePosition();

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			for (int i = 0; i < arrayLength(objects); i++) {
				if (CheckCollisionPointCircle(mousePosition, objects[i].position, objects[i].scale)) {
					shapeGrabbed = i;
					break;
				}
			}
		}

		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || !IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
			shapeGrabbed = -1;
		}

		Vector2 overlap;
		
		if (shapeGrabbed >= 0) { // Polygons
			objects[shapeGrabbed].position = mousePosition;
			float md = GetMouseWheelMove();
			objects[shapeGrabbed].scale += md / fabs(md + (md == 0)) * SCROLL_INCREMENT;
		
		}
		
		BeginDrawing();
			ClearBackground((Color){0,0,0,255});
			Polygon p = {0};
			Polygon p2 = {0};
			for (int i = 0; i < arrayLength(objects); i++) {
				Color color = (shapeGrabbed == i) ? YELLOW : WHITE;
				
				p = objects[i].shape;
				p = rotatePolygon(p, objects[i].rotation);
				p = scalePolygon(p, objects[i].scale);

				for (int e = 0; e < arrayLength(objects); e++) {
					if (e == i) continue;

//					p2 = objects[e].shape;
//					p2 = rotatePolygon(p2, objects[e].rotation);
//					p2 = scalePolygon(p2, objects[e].scale);

					overlap = (Vector2){0};
					
//					bool hit = polygonIntersect(objects[i].position, p, objects[e].position, p2, &overlap);
					bool hit = checkObjectCollision(objects[i], objects[e], &overlap);

					if (hit) {
						color = RED;
						if (i == shapeGrabbed) break;
						objects[i].position = Vector2Subtract(objects[i].position, overlap);
					}
				}

				if (objects[i].type == circle) {
					DrawCircle(objects[i].position.x, objects[i].position.y, objects[i].scale, color);
				} else {
					// Why is the x axis flipped?
					p = rotatePolygon(p, 180);
					drawPolygon(objects[i].position, p, color);
				}
			}

			DrawText("Click to grab and drag polygons.", 32, 32, 24, WHITE);
			DrawText("Scroll mouse wheel to resize current polygon.", 32, 64, 24, WHITE);

		EndDrawing();
	}

	return 0;
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
		case circle: {
			switch (obj2.type) {
				case circle:
					result = collib2d_check_circles(obj1.position.x, obj1.position.y, obj1.scale, 
													obj2.position.x, obj2.position.y, obj2.scale, 
													&overlap->x, &overlap->y);
					break;
				case rectangle:{
					// Translate rect to match most likely buggy SAT collision
					Vector2 rPos = Vector2Subtract(obj2.position, p2.vertices[2]);
					result = collib2d_check_circle_rect(obj1.position.x, obj1.position.y, obj1.scale, 
														rPos.x, rPos.y, p2.vertices[1].x, p2.vertices[3].y,
														&overlap->x, &overlap->y);
				}

					break;
				case polygon: {
					p1 = generatePolygon(MAX_POLY_SIDES); // circle
					p1 = scalePolygon(p1, obj1.scale);

					result = polygonIntersect(obj1.position, p1, obj2.position, p2, overlap);
				} break;
				
				default:
					break;
			}
		} break;
		
		// obj1 is a rectangle
		case rectangle: {
			switch (obj2.type) {
				case circle: {
					// Translate rect to match most likely buggy SAT collision
					Vector2 rPos = Vector2Subtract(obj1.position, p1.vertices[2]);

					result = collib2d_check_circle_rect(obj2.position.x, obj2.position.y, obj2.scale, 
														rPos.x, rPos.y, p1.vertices[1].x, p1.vertices[3].y,
														&overlap->x, &overlap->y);
					*overlap = Vector2Scale(*overlap, -1.0f);
				} break;
				
				case rectangle:{
					// Translate rect to match most likely buggy SAT collision
					Vector2 pos1 = Vector2Subtract(obj1.position, p1.vertices[2]);
					Vector2 pos2 = Vector2Subtract(obj2.position, p2.vertices[2]);

					result = collib2d_check_rects(	pos1.x, pos1.y, p1.vertices[1].x, p1.vertices[3].y, 
													pos2.x, pos2.y, p2.vertices[1].x, p2.vertices[3].y,
													&overlap->x, &overlap->y);
				} break;
				case polygon: {
					result = polygonIntersect(obj1.position, p1, obj2.position, p2, overlap);
				} break;
				
				default:
					break;
			}
		} break;
		
		//obj1 is a polygon
		case polygon: {
			switch (obj2.type) {
				case circle: {
					p2 = generatePolygon(MAX_POLY_SIDES); // circle
					p2 = scalePolygon(p2, obj2.scale);

					result = polygonIntersect(obj1.position, p1, obj2.position, p2, overlap);
				}
					break;
				case rectangle: {
					result = polygonIntersect(obj1.position, p1, obj2.position, p2, overlap);
				} break;
				
				case polygon: {				
					result = polygonIntersect(obj1.position, p1, obj2.position, p2, overlap);
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
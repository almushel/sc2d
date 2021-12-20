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

bool checkObjectCollision(Object obj1, Object obj2, Vector2* overlap);
static void drawObjectLabel(Vector2 pos, int type);

static Vector2 getRandomScreenCoords() {
	Vector2 result = {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT)};
	return result;
}

int main() {

	Polygon triangle = generatePolygon(3);

	Polygon square = generatePolygon(4);
	square = rotatePolygon(square, 45);
	Polygon rect = square;

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
		Vector2 overlap = {0};

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			for (int i = 0; i < arrayLength(objects); i++) {
				if (collib2d_check_point_circle(mousePosition.x, mousePosition.y, 
												objects[i].position.x, objects[i].position.y, objects[i].scale, 
												&overlap.x, &overlap.y)) {
					objects[i].position = Vector2Add(objects[i].position, overlap);
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
						//TraceLog(LOG_INFO, "overlap.x: %f, overlap.y: %f\n", overlap.x, overlap.y);
					}
				}

				if (objects[i].type == circle) {
					DrawCircle(objects[i].position.x, objects[i].position.y, objects[i].scale, color);
				} else {
					p = objects[i].shape;
					p = rotatePolygon(p, objects[i].rotation);
					p = scalePolygon(p, objects[i].scale);
					// Why is this flipped 180 degrees?
					p = rotatePolygon(p, 180);
					drawPolygon(objects[i].position, p, color);
				}
				drawObjectLabel(objects[i].position, objects[i].type);
			}

			DrawText("Click to grab and drag polygons.", 32, 32, 24, WHITE);
			DrawText("Scroll mouse wheel to resize current polygon.", 32, 64, 24, WHITE);

		EndDrawing();
	}

	return 0;
}

static void drawObjectLabel(Vector2 pos, int type) {
	switch(type) {
		case circle:
			DrawText("Circle", pos.x, pos.y, 16, WHITE);
			break;
		case rectangle:
			DrawText("Rectangle", pos.x, pos.y, 16, WHITE);
			break;
		case polygon:
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
		case circle: {
			switch (obj2.type) {
				case circle:
					result = collib2d_check_circles(obj1.position.x, obj1.position.y, obj1.scale, 
													obj2.position.x, obj2.position.y, obj2.scale, 
													&overlap->x, &overlap->y);
					break;
				case rectangle:{
					// Translate rect to match most likely buggy SAT collision
					Rectangle rect = polyToRect(obj2.position, p2);
					result = collib2d_check_circle_rect(obj1.position.x, obj1.position.y, obj1.scale, 
														rect.x, rect.y, rect.width, rect.height,
														&overlap->x, &overlap->y);
				}

					break;
				case polygon: {
					p1 = generatePolygon(MAX_POLY_SIDES); // circle
					p1 = scalePolygon(p1, obj1.scale);

					//result = polygonIntersect(obj1.position, p1, obj2.position, p2, overlap);
					result = collib2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vertCount * 2, 
													obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vertCount * 2,
													&overlap->x, &overlap->y);
				} break;
				
				default:
					break;
			}
		} break;
		
		// obj1 is a rectangle
		case rectangle: {
			switch (obj2.type) {
				case circle: {
					Rectangle rect = polyToRect(obj1.position, p1);

					result = collib2d_check_circle_rect(obj2.position.x, obj2.position.y, obj2.scale, 
														rect.x, rect.y, rect.width, rect.height,
														&overlap->x, &overlap->y);
					*overlap = Vector2Scale(*overlap, -1.0f); // Flip the overlap, because the circle is the second object
				} break;
				
				case rectangle:{
					Rectangle rect1 = polyToRect(obj1.position, p1);
					Rectangle rect2 = polyToRect(obj2.position, p2);

					result = collib2d_check_rects(	rect1.x, rect1.y, rect1.width, rect1.height, 
													rect2.x, rect2.y, rect2.width, rect2.height,
													&overlap->x, &overlap->y);
				} break;
				case polygon: {
//					result = polygonIntersect(obj1.position, p1, obj2.position, p2, overlap);
					result = collib2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vertCount * 2, 
													obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vertCount * 2,
													&overlap->x, &overlap->y);
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

					result = collib2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vertCount * 2, 
													obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vertCount * 2,
													&overlap->x, &overlap->y);
				}
					break;
				case rectangle: {
					result = collib2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vertCount * 2, 
													obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vertCount * 2,
													&overlap->x, &overlap->y);
				} break;
				
				case polygon: {				
					result = collib2d_check_poly2d(	obj1.position.x, obj1.position.y, (float*)p1.vertices, p1.vertCount * 2, 
													obj2.position.x, obj2.position.y, (float*)p2.vertices, p2.vertCount * 2,
													&overlap->x, &overlap->y);
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
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
	Vector2 position;
	float scale;
	float rotation;
	Polygon shape;
	bool hit;
} Object;

float normalizeDegrees(float degrees);

static Vector2 getRandomScreenCoords() {
	Vector2 result = {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT)};
	return result;
}

int main() {

	Polygon triangle = generatePolygon(3);
	Polygon square = generatePolygon(4);
	square = rotatePolygon(square, 45);
	Polygon pentagon = generatePolygon(5);
	pentagon = rotatePolygon(pentagon, -90);
	Polygon hexagon = generatePolygon(6);
	Polygon octagon = generatePolygon(8);

	Object polygons[] = {
		{
			.shape = triangle,
			.scale = POLYGON_SCALE,
		},

		{
			.shape = square,
			.scale = POLYGON_SCALE,
		},

		{
			.shape = pentagon,
			.scale = POLYGON_SCALE,
		},

		{
			.shape = hexagon,
			.scale = POLYGON_SCALE,
		},

		{
			.shape = octagon,
			.scale = POLYGON_SCALE,
		}
	};

	Vector2 c1Pos = getRandomScreenCoords();
	Vector2 c2Pos = getRandomScreenCoords();
	float c1Radius = 10.0f;
	float c2Radius = 12.0f;
	Color c1Color = WHITE;
	Color c2Color = WHITE;

	Rectangle rect1 = {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT), 50, 50};
	Rectangle rect2 = {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT), 50, 50};
	Color r1Color = WHITE;
	Color r2Color = WHITE;

	for (int i = 0; i < arrayLength(polygons); i++) {
		GetRandomValue(0, SCREEN_WIDTH);
		polygons[i].position = getRandomScreenCoords();
	}


	int shapeGrabbed = -1;
	
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Separating Axis Theorem");
	
	while(!WindowShouldClose()) {

		Vector2 mousePosition = GetMousePosition();
		c1Color = c2Color = WHITE;
		r1Color = r2Color = WHITE;

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			for (int i = 0; i < arrayLength(polygons); i++) {
				if (CheckCollisionPointCircle(mousePosition, polygons[i].position, polygons[i].scale)) {
					shapeGrabbed = i;
					break;
				}
			}

			if (CheckCollisionPointCircle(mousePosition, c1Pos, c1Radius)) {
				shapeGrabbed = -2;
			}
			else if (CheckCollisionPointCircle(mousePosition, c2Pos, c2Radius)) {
				shapeGrabbed = -3;
			}
			else if (CheckCollisionPointRec(mousePosition, rect1)) {
				shapeGrabbed = -4;
			}
			else if (CheckCollisionPointRec(mousePosition, rect2)) {
				shapeGrabbed = -5;
			}
		}

		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || !IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
			shapeGrabbed = -1;
		}

		Vector2 overlap;
		
		if (shapeGrabbed >= 0) { // Polygons
			polygons[shapeGrabbed].position = mousePosition;
			float md = GetMouseWheelMove();
			polygons[shapeGrabbed].scale += md / fabs(md + (md == 0)) * SCROLL_INCREMENT;
		
		} else if (shapeGrabbed == -2) { // Circle 1
			c1Pos = mousePosition;
			if (collib2d_check_circles(c1Pos.x, c1Pos.y, c1Radius, c2Pos.x, c2Pos.y, c2Radius, &overlap.x, &overlap.y)) {
				c1Color = c2Color = RED;
				c2Pos = Vector2Add(c2Pos, overlap);
			}
		
		} else if (shapeGrabbed == -3) { // Circle 2
			c2Pos = mousePosition;
			if (collib2d_check_circles(c2Pos.x, c2Pos.y, c2Radius, c1Pos.x, c1Pos.y, c1Radius, &overlap.x, &overlap.y)) {
				c1Color = c2Color = RED;
				c1Pos = Vector2Add(c1Pos, overlap);
			}
		}

		else if (shapeGrabbed == -4) { // Rectangle 1
			rect1.x = mousePosition.x - rect1.width/2;
			rect1.y = mousePosition.y - rect1.height/2;

			bool hit = collib2d_check_rects(rect1.x, rect1.y, rect1.width, rect1.height, rect2.x, rect2.y, rect2.width, rect2.height, &overlap.x, &overlap.y);
			if (hit) {
				Vector2 newPos = Vector2Add((Vector2){rect2.x, rect2.y}, overlap);
				rect2.x = newPos.x;
				rect2.y = newPos.y;
				r1Color = r2Color = RED;
			}
		}

		else if (shapeGrabbed == -5) {
			rect2.x = mousePosition.x - rect1.width/2;
			rect2.y = mousePosition.y - rect1.height/2;

			bool hit = collib2d_check_rects(rect2.x, rect2.y, rect2.width, rect2.height, rect1.x, rect1.y, rect1.width, rect1.height, &overlap.x, &overlap.y);
			if (hit) {
				Vector2 newPos = Vector2Add((Vector2){rect1.x, rect1.y}, overlap);
				rect1.x = newPos.x;
				rect1.y = newPos.y;
				r1Color = r2Color = RED;
			}
		}
		
		BeginDrawing();
			ClearBackground((Color){0,0,0,255});
			Polygon p = {0};
			Polygon p2 = {0};
			for (int i = 0; i < arrayLength(polygons); i++) {
				Color color = (shapeGrabbed == i) ? YELLOW : WHITE;
				
				p = polygons[i].shape;
				p = rotatePolygon(p, polygons[i].rotation);
				p = scalePolygon(p, polygons[i].scale);

				for (int e = 0; e < arrayLength(polygons); e++) {
					if (e == i) continue;

					p2 = polygons[e].shape;
					p2 = rotatePolygon(p2, polygons[e].rotation);
					p2 = scalePolygon(p2, polygons[e].scale);

					overlap = (Vector2){0};
					
					bool hit = polygonIntersect(polygons[i].position, p, polygons[e].position, p2, &overlap);

					if (hit) {
						color = RED;
						if (i == shapeGrabbed) break;
						polygons[i].position = Vector2Subtract(polygons[i].position, overlap);
					}
				}

				// Why is the x axis flipped?
				p = rotatePolygon(p, 180);
				drawPolygon(polygons[i].position, p, color);
			}

			DrawCircle(c1Pos.x, c1Pos.y, c1Radius, c1Color);
			DrawCircle(c2Pos.x, c2Pos.y, c2Radius, c2Color);

			DrawRectangleRec(rect1, r1Color);
			DrawRectangleRec(rect2, r2Color);

			DrawText("Click to grab and drag polygons.", 32, 32, 24, WHITE);
			DrawText("Scroll mouse wheel to resize current polygon.", 32, 64, 24, WHITE);

		EndDrawing();
	}

	return 0;
}
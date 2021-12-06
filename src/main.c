#include "raylib.h"
#include "raymath.h"
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

	for (int i = 0; i < arrayLength(polygons); i++) {
		GetRandomValue(0, SCREEN_WIDTH);
		polygons[i].position = (Vector2){GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT)};
	}


	int shapeGrabbed = -1;
	
	SetTargetFPS(60);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Separating Axis Theorem");
	
	while(!WindowShouldClose()) {

		Vector2 mousePosition = GetMousePosition();

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			for (int i = 0; i < arrayLength(polygons); i++) {
				if (CheckCollisionPointCircle(mousePosition, polygons[i].position, polygons[i].scale)) {
					shapeGrabbed = i;
					break;
				}
			}
		}

		if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || !IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
			shapeGrabbed = -1;
		}

		if (shapeGrabbed >= 0) {
			polygons[shapeGrabbed].position = mousePosition;
			float md = GetMouseWheelMove();
			polygons[shapeGrabbed].scale += md / fabs(md + (md == 0)) * SCROLL_INCREMENT;
		}
		
		BeginDrawing();
			ClearBackground((Color){0,0,0,255});
			Polygon p = {0};
			Polygon p2 = {0};
			for (int i = 0; i < arrayLength(polygons); i++) {
				//polygons[i].rotation++;
				//polygons[i].rotation = normalizeDegrees(polygons[i].rotation);

				Color color = (shapeGrabbed == i) ? YELLOW : WHITE;
				
				p = polygons[i].shape;
				p = rotatePolygon(p, polygons[i].rotation);
				p = scalePolygon(p, polygons[i].scale);

				Vector2 overlap;
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

			DrawText("Click to grab and drag polygons.", 32, 32, 24, WHITE);
			DrawText("Scroll mouse wheel to resize current polygon.", 32, 64, 24, WHITE);

		EndDrawing();
	}

	return 0;
}
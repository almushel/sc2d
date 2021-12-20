#include "raylib.h"

#define MAX_POLY_SIDES 12

typedef struct Polygon {
	Vector2 vertices[MAX_POLY_SIDES];
	int vertCount;
} Polygon;

void logPolyVerts(Polygon p);
void drawPolygon(Vector2 v, Polygon p, Color c);

Polygon generatePolygon(int sides);
Rectangle polyToRect(Vector2 position, Polygon p);
Polygon scalePolygon(Polygon p, float scale);
Polygon translatePolygon(Polygon p, Vector2 translation);
Polygon rotatePolygon(Polygon p, float degrees);
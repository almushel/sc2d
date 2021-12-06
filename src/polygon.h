#include "raylib.h"

typedef struct Polygon {
	Vector2 vertices[12];
	int vertCount;
} Polygon;

typedef struct Range {
	float min, max;
} Range;

Vector2 Vector2Normal(Vector2 v, bool clockwise);

void logPolyVerts(Polygon p);
void drawPolygon(Vector2 v, Polygon p, Color c);

Polygon generatePolygon(int sides);
Polygon scalePolygon(Polygon p, float scale);
Polygon translatePolygon(Polygon p, Vector2 translation);
Polygon rotatePolygon(Polygon p, float degrees);
bool polygonIntersect(Vector2 v1, Polygon p1, Vector2 v2, Polygon p2, Vector2* overlap);
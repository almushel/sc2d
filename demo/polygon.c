#include "raylib.h"
#include "raymath.h"
#include "polygon.h"

void logPolyVerts(Polygon p) {
	for (int i = 0; i < p.vertCount; i++) {
		TraceLog(LOG_INFO, "Vertex %d X: %f, Y: %f\n", i, p.vertices[i].x, p.vertices[i].y);
	}
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

float normalizeDegrees(float degrees) {
	float result = degrees;
	while (result > 360.0f) result -= 360.0f;
	while (result < 0.0f) result += 360.0f;
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
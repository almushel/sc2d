#include "raylib.h"
#include "raymath.h"
#include "polygon.h"

Vector2 Vector2Normal(Vector2 v, bool clockwise) {
	Vector2 result = 	clockwise ?
						(Vector2){v.y, -v.x} :
						(Vector2){-v.y, v.x};
	return result;
}

void logPolyVerts(Polygon p) {
	for (int i = 0; i < p.vertCount; i++) {
		TraceLog(LOG_INFO, "Vertex %d X: %f, Y: %f\n", i, p.vertices[i].x, p.vertices[i].y);
	}
}

void drawPolygon(Vector2 v, Polygon p, Color c) {
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

static Range projectPolygonToAxis(Vector2 axis, Polygon p) {
	Range result = {0};

	for (int i = 0; i < p.vertCount; i++) {
		float dot = Vector2DotProduct(p.vertices[i], axis);
		result.min = fmin(result.min, dot);
		result.max = fmax(result.max, dot);
	}

	return result;
}

bool polygonIntersect(Vector2 v1, Polygon p1, Vector2 v2, Polygon p2, Vector2* overlap) {
	bool result = true;
	Range p1Projection, p2Projection;
	Vector2 axis;
	Vector2 delta = {v2.x - v1.x, v2.y - v1.y};
	float offset = 0;
	float minDistance = INFINITY;

	// First polygon
	for (int i = 0; i < p1.vertCount; i++) {
		axis = getPolygonEdge(p1, i);
		axis = Vector2Normal(axis, true);
		axis = Vector2Normalize(axis);
		offset = Vector2DotProduct(axis, delta);

		p1Projection = projectPolygonToAxis(axis, p1);
		p1Projection.min += offset;
		p1Projection.max += offset;
		p2Projection = projectPolygonToAxis(axis, p2);
		
		if ( (p1Projection.min > p2Projection.max) || (p1Projection.max < p2Projection.min)) {
			return false;
		}
		
		float distance = fmin(p1Projection.max, p2Projection.max) - fmax(p1Projection.min, p2Projection.min);
		if (distance < minDistance) {
			minDistance = distance;
			*overlap = Vector2Scale(axis, offset > 0 ? 1 : -1);
		}
	}

	// Second Polygon
	for (int i = 0; i < p2.vertCount; i++) {
		axis = getPolygonEdge(p2, i);
		axis = Vector2Normal(axis, true);
		axis = Vector2Normalize(axis);
		offset = Vector2DotProduct(axis, delta);

		p1Projection = projectPolygonToAxis(axis, p1);
		p1Projection.min += offset;
		p1Projection.max += offset;
		p2Projection = projectPolygonToAxis(axis, p2);
		
		if ( (p1Projection.min > p2Projection.max) || (p1Projection.max < p2Projection.min)) {
			return false;
		}
		

		float distance = fmin(p1Projection.max, p2Projection.max) - fmax(p1Projection.min, p2Projection.min);
		if (distance < minDistance) {
			minDistance = distance;
			*overlap = Vector2Scale(axis, offset > 0 ? 1 : -1);
		}
	}

	*overlap = Vector2Scale(*overlap, minDistance);

	return result;
}
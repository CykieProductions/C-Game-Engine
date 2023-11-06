#pragma once
#include "Cytools.h"
#include "Vectors.h"
#include "Include/cute_c2.h"
//#include "DriverSystem.h"


//! STRUCTS //
//OLD
/*typedef struct LineSegment
{
	Vector2 start;
	Vector2 end;
	Vector2 normal;

} LineSegment;

typedef struct Collision_Point
{
	bool collided;
	Vector2 point;
	Vector2 crosses[64];
	uint timesCrossed;

	LineSegment edges[64];
	uint numOfEdges;

} Collision_Point;

typedef struct Collision_Poly
{
	Collision_Point collisions[96];
	uint numOfCollisions;

} Collision_Poly;*/

typedef struct Collision
{
	bool collided;
	c2Manifold manifold;
	Vector2 resolve;

} Collision;

//! FUNCTIONS //
//OLD
/*
//https://stackoverflow.com/questions/11716268/point-in-polygon-algorithm#:~:text=By%20repeatedly%20inverting%20the%20value%20of%20c%2C%20the,if%20an%20even%20number%2C%20the%20point%20is%20outside.
Collision_Point PointInPolygon(Vector2 point, Vector2 verts[], uint numOfVerts);

Collision_Poly PolygonInPolygon(Vector2 first[], uint numInFirst, Vector2 second[], uint numInSecond);
Collision_Point PolygonInPolyTrigger(Vector2 first[], uint numInFirst, Vector2 second[], uint numInSecond);
//*/

Vector2 C2VToVec2(c2v vec);
#pragma once
#include <stdio.h>

typedef struct Vector2
{
	float x;
	float y;

} Vector2;
typedef struct Vector3
{
	float x;
	float y;
	float z;

} Vector3;
typedef struct Vector4
{
	float x;
	float y;
	float z;
	float w;

} Vector4;

typedef struct Vector2INT
{
	int x;
	int y;

} Vector2INT;
typedef struct Vector2LONG
{
	long x;
	long y;

} Vector2LONG;

//Functions//

Vector2INT Vec2ToVec2INT(Vector2 vec2);
Vector2 Vec2INTToVec2(Vector2INT vec2);
Vector2 Vec2Multiply(Vector2 target, Vector2 factor);
Vector2 Vec2MultiplyF(Vector2 target, float factor);
Vector2 Vec2Add(Vector2 first, Vector2 second);
float Vec2Dot(Vector2 first, Vector2 second);
float Vec2Magnitude(const Vector2* vec);
void Vec2Normalize(_Inout_ Vector2* vec);
float Vec2Distance(Vector2 first, Vector2 second);

Vector3 Vec3Multiply(_In_ const Vector3* target, Vector3 factor);
Vector3 Vec3MultiplyF(_In_ const Vector3* target, float factor);
Vector3 Vec3Add(_In_ const Vector3* target, Vector3 factor);
float Vec3Dot(Vector3 first, Vector3 second);
float Vec3Magnitude(const Vector3* vec);
void Vec3Normalize(_Inout_ Vector3* vec);

Vector4 Vec4MultiplyF(_In_ const Vector4* target, float factor);
Vector4 Vec4Add(_In_ const Vector4* first, Vector4 second);

//Conversions

Vector4 Vec4FromVec3(_In_ const Vector3* input, float w);
Vector3 Vec3FromVec4(_In_ const Vector4* input);
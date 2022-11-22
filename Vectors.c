#include "Cytools.h"
#include "Vectors.h"


#pragma region Vector2 Functions
Vector2INT Vec2ToVec2INT(Vector2 vec2)
{
    Vector2INT result = { 0 };
    result.x = vec2.x;
    result.y = vec2.y;
    return result;
}
Vector2 Vec2INTToVec2(Vector2INT vec2)
{
    Vector2 result = { 0 };
    result.x = vec2.x;
    result.y = vec2.y;
    return result;
}

Vector2 Vec2Multiply(Vector2 target, Vector2 factor)
{
    return (Vector2) {
        target.x * factor.x,
        target.y * factor.y
    };
}

Vector2 Vec2MultiplyF(Vector2 target, float factor)
{
    return (Vector2) {
        target.x * factor,
        target.y * factor
    };
}
Vector2 Vec2Add(Vector2 first, Vector2 second)
{
    return (Vector2) {
        first.x + second.x,
        first.y + second.y
    };
}

float Vec2Dot(Vector2 first, Vector2 second)
{
    return (first.x * second.x) + (first.y * second.y);
}

float Vec2Magnitude(const Vector2* vec)
{
    return sqrt(vec->x * vec->x + vec->y * vec->y);
}

void Vec2Normalize(_Inout_ Vector2* vec)
{
    float magnitude = Vec2Magnitude(vec);
    if (magnitude != 0)
    {
        vec->x /= magnitude;
        vec->y /= magnitude;
    }
    else
    {
        vec->x = 0;
        vec->y = 0;
    }
}

float Vec2Distance(Vector2 first, Vector2 second)
{
    return sqrt(pow(second.x - first.x, 2) + pow(second.y - first.y, 2));
}
#pragma endregion


#pragma region Vector3 Functions
Vector3 Vec3Multiply(_In_ const Vector3* target, Vector3 factor)
{
    return (Vector3) {
        target->x * factor.x,
        target->y * factor.y,
        target->z * factor.z
    };
}
Vector3 Vec3MultiplyF(_In_ const Vector3* target, float factor)
{
    return (Vector3) {
        target->x * factor,
        target->y * factor,
        target->z * factor
    };
}

Vector3 Vec3Add(_In_ const Vector3* first, Vector3 second)
{
    return (Vector3) {
        first->x + second.x,
        first->y + second.y,
        first->z + second.z
    };
}

float Vec3Dot(Vector3 first, Vector3 second)
{
    return (first.x * second.x) + (first.y * second.y) + (first.z * second.z);
}

float Vec3Magnitude(const Vector3* vec)
{
    return sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
}

void Vec3Normalize(_Inout_ Vector3* vec)
{
    float magnitude = Vec3Magnitude(vec);
    vec->x /= magnitude;
    vec->y /= magnitude;
    vec->z /= magnitude;
}
#pragma endregion

Vector4 Vec4FromVec3(_In_ const Vector3* input, float w)
{
    return (Vector4) {
        input->x,
        input->y,
        input->z,
        w
    };
}
Vector3 Vec3FromVec4(_In_ const Vector4* input)
{
    return (Vector3) {
        input->x,
        input->y,
        input->z
    };
}

Vector4 Vec4MultiplyF(_In_ const Vector4* target, float factor)
{
    return (Vector4) {
        target->x * factor,
        target->y * factor,
        target->z * factor,
        target->w * factor
    };
}

Vector4 Vec4Add(_In_ const Vector4* first, Vector4 second)
{
    return (Vector4) {
        first->x + second.x,
        first->y + second.y,
        first->z + second.z,
        first->w + second.w
    };
}
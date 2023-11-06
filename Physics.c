#include "Physics.h"
/*
Collision_Point PointInPolygon(Vector2 point, Vector2 verts[], uint numOfVerts)
{
    int i, j = 0;
    bool result = false;
    //x bool hasCrossed = false;
    Collision_Point info = { 0 };
    info.point = point;

    //! <j> is the last point connected to <i>
    for (i = 0, j = numOfVerts - 1; i < numOfVerts; j = i++) 
    {
        float zeroCheck = (verts[j].y - verts[i].y);
        zeroCheck = zeroCheck == 0 ? .00001f : zeroCheck;
        
        //Check for crosses
        if ((verts[i].y > point.y) != (verts[j].y > point.y) &&
            point.x < (verts[j].x - verts[i].x) * (point.y - verts[i].y) / zeroCheck + verts[i].x)
        {
            //x if (!hasCrossed)//First cross
            //x hasCrossed = true;

            info.crosses[info.timesCrossed] = verts[i];
            info.timesCrossed++;

            result = !result;
        }

        //Get all normals
        info.edges[i] = (LineSegment){ verts[j], verts[i] };
        float dx, dy;
        dx = info.edges[i].end.x - info.edges[i].start.x;
        dy = info.edges[i].end.y - info.edges[i].start.y;

        info.edges[i].normal = (Vector2){ dy, -dx };
        Vec2Normalize(&info.edges[i].normal);
    }
    info.numOfEdges = numOfVerts;

    info.collided = result;
    return info;
}

Collision_Poly PolygonInPolygon(Vector2 firstPoly[], uint numInFirst, Vector2 secondPoly[], uint numInSecond)
{
    Collision_Point info = { 0 };
    Collision_Poly polyCol = { 0 };
    for (size_t i = 0; i < numInSecond; i++)
    {
        info = PointInPolygon(secondPoly[i], firstPoly, numInFirst);
        if (info.collided)
        {
            polyCol.collisions[polyCol.numOfCollisions] = info;//collided
            polyCol.numOfCollisions++;
        }
    }
    for (size_t i = 0; i < numInSecond; i++)
    {
        info = PointInPolygon(firstPoly[i], secondPoly, numInSecond);
        if (info.collided)
        {
            polyCol.collisions[polyCol.numOfCollisions] = info;//collided
            polyCol.numOfCollisions++;
        }
    }

    return polyCol;
}

Collision_Point PolygonInPolyTrigger(Vector2 firstPoly[], uint numInFirst, Vector2 secondPoly[], uint numInSecond)
{
    Collision_Point info = { 0 };
    for (size_t i = 0; i < numInSecond; i++)
    {
        info = PointInPolygon(secondPoly[i], firstPoly, numInFirst);
        if (info.collided)
        {
            return info;//collided
        }
    }
    for (size_t i = 0; i < numInSecond; i++)
    {
        info = PointInPolygon(firstPoly[i], secondPoly, numInSecond);
        if (info.collided)
        {
            return info;//collided
        }
    }

    return info;
}

//*/

Vector2 C2VToVec2(c2v vec)
{
    return (Vector2) { vec.x, vec.y };
}

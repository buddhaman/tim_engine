
// TODO: Better rand.
R32
RandomR32(R32 min, R32 max)
{
    return min + (max - min)*(rand()%RAND_MAX)/RAND_MAX;
}

int
RandomUI32(U32 min, U32 max)
{
    return min + (rand()%(max-min));
}

R32
Sign(R32 x)
{
    return x < 0 ? -1.0 : 1.0;
}

Vec2
RandomNormalPair()
{
    R32 x, y, s;
    do
    {
        x = RandomR32(-1, 1);
        y = RandomR32(-1, 1);
        s = x*x + y*y;
    } while(s > 1.0);
    R32 factor = sqrtf(-2*logf(s)/s);
    return V2(factor*x, factor*y);
}

internal inline Vec2
V2Polar(R32 angle, R32 length)
{
    return V2(cosf(angle)*length, sinf(angle)*length);
}

internal inline R32 
Lerp(R32 a, R32 b, R32 factor)
{
    return (1.0-factor)*a+factor*b;
}

Vec3 
GetZIntersection(Vec3 rayPos, Vec3 rayDir, R32 z)
{
    R32 zDiff = z-rayPos.z;
    R32 s = zDiff/rayDir.z;
    return V3Add(rayPos, V3MulS(rayDir, s));
}

internal inline B32
BoxPoint2Intersect(Vec2 pos, Vec2 dims, Vec2 point)
{
    R32 xDiff = point.x-pos.x-dims.x/2;
    R32 yDiff = point.y-pos.y-dims.y/2;
    return (fabsf(xDiff) < dims.x/2 && fabsf(yDiff) < dims.y/2);
}

internal inline B32
OrientedBoxPoint2Intersect(Vec2 pos, Vec2 dims, R32 angle, Vec2 point)
{
    // Project onto axes see if dot < dims
    R32 c = cosf(angle);
    R32 s = sinf(angle);
    R32 xDiff = point.x-pos.x;
    R32 yDiff = point.y-pos.y;
    R32 xProj = c*xDiff + s*yDiff;
    R32 yProj = -s*xDiff + c*yDiff;
    return (fabsf(xProj) < dims.x/2 && fabsf(yProj) < dims.y/2);
}

internal inline Vec2
GetCoordinateInBox(Vec2 pos, Vec2 dims, R32 angle, Vec2 point)
{
    R32 c = cosf(angle);
    R32 s = sinf(angle);
    R32 xDiff = point.x-pos.x;
    R32 yDiff = point.y-pos.y;
    R32 xProj = c*xDiff + s*yDiff;
    R32 yProj = -s*xDiff + c*yDiff;
    return V2(xProj/dims.x+0.5, yProj/dims.y+0.5);
}

internal inline B32
CirclePointIntersect(Vec2 pos, R32 radius, Vec2 point)
{
    Vec2 diff = V2Sub(point, pos);
    return V2Len2(diff) <= radius*radius;
}

// Ignores points inside the rectangle.
internal inline R32 
GetNearestBoxEdgeLocation(Vec2 pos, Vec2 dims, R32 angle, Vec2 point, BoxEdgeLocation *location)
{
    R32 c = cosf(angle);
    R32 s = sinf(angle);
    R32 xDiff = point.x-pos.x;
    R32 yDiff = point.y-pos.y;
    Vec2 p = V2(c*xDiff + s*yDiff, -s*xDiff + c*yDiff);   // point coordinates seen from box transform.

    R32 result = -1.0;
    *location = (BoxEdgeLocation){};

    B32 inYRange = fabsf(p.x) < dims.x/2.0;
    B32 inXRange = fabsf(p.y) < dims.y/2.0;
    if(inYRange)
    {
        if(!inXRange)
        {
            int sign = Sign(p.y);
            R32 yPos = sign*dims.y/2.0;
            location->pos = V2(p.x, yPos);
            location->yEdge = sign;
            location->offset = 1.0-sign*(p.x+sign*dims.x/2)/dims.x;
            result = fabsf(p.y-yPos);
        }
        else
        {
            // Inside rectangle
        }
    }
    else if(inXRange)
    {
        int sign = Sign(p.x);
        R32 xPos = sign*dims.x/2.0;
        location->pos = V2(xPos, p.y);
        location->xEdge = sign;
        location->offset = sign*(p.y+sign*dims.y/2)/dims.y;
        result = fabsf(p.x-xPos);
    }
    else
    {
        int xSign = Sign(p.x);
        int ySign = Sign(p.y);
        Vec2 corner = V2(xSign*dims.x/2.0, ySign*dims.y/2.0);
        location->pos = corner;
        location->xEdge = xSign==ySign ? xSign : 0;
        location->yEdge = xSign==ySign ? 0 : ySign;
        location->offset = 1.0;
        result = V2Dist(corner, p);
    }

    // Transform result back to rotated rectangle.
    location->pos = V2(
            pos.x + c*location->pos.x - s*location->pos.y,
            pos.y + s*location->pos.x + c*location->pos.y
            );

    return result;
}

internal inline Vec2
GetBoxEdgePosition(Vec2 pos, Vec2 dims, R32 angle, int xEdge, int yEdge, R32 offset)
{
    Vec2 result;
    if(xEdge!=0)
    {
        int ySign = xEdge;
        result.x = xEdge*dims.x/2;
        result.y = ySign*offset*dims.y-ySign*dims.y/2;
    }
    else
    {
        int xSign = -yEdge;
        result.x = xSign*offset*dims.x-xSign*dims.x/2;
        result.y = yEdge*dims.y/2;
    }
    // Transform to rotated rectangle.
    result = V2Add(pos, V2Rotate(result, angle));
    return result;
}

internal inline Vec4
RGBAToVec4(U32 hex)
{
    return V4( (hex >> 24 & 255)/255.0f,
            (hex >> 16 & 255)/255.0f,
            (hex >> 8 & 255)/255.0f,
            (hex & 255)/255.0f);
}

internal inline U32
Vec4ToRGBA(Vec4 color)
{
    int r = (color.x*255);
    int g = (color.y*255);
    int b = (color.z*255);
    int a = (color.w*255);
    return (a << 24) + (b << 16) + (g << 8) + r;
}

internal inline Vec4
ShadeRGB(Vec4 color, R32 shade)
{
    return V4(shade*color.r, shade*color.g, shade*color.b, 1.0f);
}

internal inline R32
RadToDeg(R32 rad)
{
    return (180 * rad / M_PI);
}

internal inline R32
DegToRad(R32 rad)
{
    return rad*(M_PI/180.0);
}

internal inline R32
NormalizeAngle(R32 rad)
{
    rad = fmodf(rad, M_PI*2);
    rad = fmodf(rad+M_PI*2, M_PI*2);
    if(rad > M_PI)
    {
        rad-=M_PI*2;
    }
    return rad;
}

internal inline R32
GetNormalizedAngDiff(R32 a, R32 b)
{
    R32 diff = a-b;
    return NormalizeAngle(diff);
}

internal inline R32
ClampF(R32 min, R32 max, R32 x)
{
    return x < min ? min : (x > max ? max : x);
}

// TODO: maybe move somewhere else

internal inline size_t
IndexOfElement(void *array, size_t elementSize, void *element)
{
    size_t index = ((size_t)element - (size_t)array)/elementSize;
    Assert(index >= 0);
    return index;
}

internal inline void
ArrayRemoveElement(void *array, size_t elementSize, size_t nElements, void *element)
{
    size_t index = IndexOfElement(array, elementSize, element);
    size_t nMoveElements = nElements-index;
    if(nMoveElements > 0)
    {
        memmove(element, (U8*)element+elementSize, nMoveElements*elementSize);
    }
}


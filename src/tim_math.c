
// TODO: Better random.
r32
RandomR32(r32 min, r32 max)
{
    return min + (max - min)*(random()%RAND_MAX)/RAND_MAX;
}

Vec2
RandomNormalPair()
{
    r32 x, y, s;
    do
    {
        x = RandomR32(-1, 1);
        y = RandomR32(-1, 1);
        s = x*x + y*y;
    } while(s > 1.0);
    r32 factor = sqrtf(-2*logf(s)/s);
    return vec2(factor*x, factor*y);
}

Vec3 
GetZIntersection(Vec3 rayPos, Vec3 rayDir, r32 z)
{
    r32 zDiff = z-rayPos.z;
    r32 s = zDiff/rayDir.z;
    return v3_add(rayPos, v3_muls(rayDir, s));
}

internal inline b32
OrientedBoxPoint2Intersect(Vec2 pos, Vec2 dims, r32 angle, Vec2 point)
{
    // Project onto axes see if dot < dims
    r32 c = cosf(angle);
    r32 s = sinf(angle);
    r32 xDiff = point.x-pos.x;
    r32 yDiff = point.y-pos.y;
    r32 xProj = c*xDiff + s*yDiff;
    r32 yProj = -s*xDiff + c*yDiff;
    return (fabsf(xProj) < dims.x/2 && fabsf(yProj) < dims.y/2);
}

internal inline b32
CirclePointIntersect(Vec2 pos, r32 radius, Vec2 point)
{
    Vec2 diff = v2_sub(point, pos);
    return v2_length2(diff) <= radius*radius;
}

internal inline Vec4
RGBAToVec4(ui32 hex)
{
    return vec4( (hex >> 24 & 255)/255.0f,
            (hex >> 16 & 255)/255.0f,
            (hex >> 8 & 255)/255.0f,
            (hex & 255)/255.0f);
}


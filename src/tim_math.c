
// TODO: Better random.
r32
RandomR32(r32 min, r32 max)
{
    return min + (max - min)*(random()%10000)/10000.0;
}

Vec3 
GetZIntersection(Vec3 rayPos, Vec3 rayDir, r32 z)
{
    r32 zDiff = z-rayPos.z;
    r32 s = zDiff/rayDir.z;
    return v3_add(rayPos, v3_muls(rayDir, s));
}

internal inline Vec4
ARGBToVec4(ui32 hex)
{
    return vec4( (hex >> 24 & 255)/255.0f,
            (hex >> 16 & 255)/255.0f,
            (hex >> 8 & 255)/255.0f,
            (hex & 255)/255.0f);
}


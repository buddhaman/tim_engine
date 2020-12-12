
Vec3 
GetZIntersection(Vec3 rayPos, Vec3 rayDir, r32 z)
{
    r32 zDiff = z-rayPos.z;
    r32 s = zDiff/rayDir.z;
    return v3_add(rayPos, v3_muls(rayDir, s));
}

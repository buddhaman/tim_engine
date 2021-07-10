
b32
NKEditFloatProperty(struct nk_context *ctx, 
        char *label, 
        r32 min, 
        r32 *value, 
        r32 max, 
        r32 stepSize, 
        r32 incPerPixel)
{
    r32 v = *value;
    nk_property_float(ctx, label, min, &v, max, stepSize, incPerPixel);
    if(v!=*value)
    {
        *value = v;
        return 1;
    }
    return 0;
}

b32
NKEditFloatPropertyWithTooltip(struct nk_context *ctx, 
        char *label, 
        char *tooltip,
        r32 min, 
        r32 *value, 
        r32 max, 
        r32 stepSize, 
        r32 incPerPixel)
{
    if(nk_widget_is_hovered(ctx))
    {
        nk_tooltip(ctx, tooltip);
    }
    return NKEditFloatProperty(ctx, label, min, value, max, stepSize, incPerPixel);
}

b32
NKEditRadInDegProperty(struct nk_context *ctx, 
        char *label, 
        r32 minRad, 
        r32 *rad, 
        r32 maxRad, 
        r32 stepSize, 
        r32 incPerPixel)
{
    r32 valInDeg = RadToDeg(*rad);
    r32 v = valInDeg;
    nk_property_float(ctx, label, RadToDeg(minRad), &v, RadToDeg(maxRad), stepSize, incPerPixel);
    if(v!=valInDeg)
    {
        *rad = DegToRad(v);
        return 1;
    }
    return 0;
}





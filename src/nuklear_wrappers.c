
B32
NKEditFloatProperty(struct nk_context *ctx, 
        char *label, 
        R32 min, 
        R32 *value, 
        R32 max, 
        R32 stepSize, 
        R32 incPerPixel)
{
    R32 v = *value;
    nk_property_float(ctx, label, min, &v, max, stepSize, incPerPixel);
    if(v!=*value)
    {
        *value = v;
        return 1;
    }
    return 0;
}

B32
NKEditFloatPropertyWithTooltip(struct nk_context *ctx, 
        char *label, 
        char *tooltip,
        R32 min, 
        R32 *value, 
        R32 max, 
        R32 stepSize, 
        R32 incPerPixel)
{
    if(nk_widget_is_hovered(ctx))
    {
        nk_tooltip(ctx, tooltip);
    }
    return NKEditFloatProperty(ctx, label, min, value, max, stepSize, incPerPixel);
}

B32
NKEditRadInDegProperty(struct nk_context *ctx, 
        char *label, 
        R32 minRad, 
        R32 *rad, 
        R32 maxRad, 
        R32 stepSize, 
        R32 incPerPixel)
{
    R32 valInDeg = RadToDeg(*rad);
    R32 v = valInDeg;
    nk_property_float(ctx, label, RadToDeg(minRad), &v, RadToDeg(maxRad), stepSize, incPerPixel);
    if(v!=valInDeg)
    {
        *rad = DegToRad(v);
        return 1;
    }
    return 0;
}





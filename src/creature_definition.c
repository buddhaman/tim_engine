
ui32
GetIndexOfBodyPart(CreatureDefinition *def, ui32 id)
{
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(part->id == id) return bodyPartIdx;
    }
    return -1;
}

BodyPartDefinition *
GetBodyPartById(CreatureDefinition *def, ui32 id)
{
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(part->id == id) return part;
    }
    return NULL;
}

r32
GetLocalAngleFromAbsoluteAngle(CreatureDefinition *def,
        BodyPartDefinition *part,
        r32 absAngle)
{
    r32 edgeAngle = atan2f(part->yEdge, part->xEdge);
    BodyPartDefinition *parent = GetBodyPartById(def, part->connectionId);
    return NormalizeAngle(absAngle - parent->angle -edgeAngle);
}

r32 
GetAbsoluteEdgeAngle(BodyPartDefinition *part, int xEdge, int yEdge)
{
    return NormalizeAngle(atan2f(yEdge, xEdge)+part->angle);
}

ui32
GetSubNodeBodyPartsById(CreatureDefinition *def, 
        BodyPartDefinition *parent,
        ui32 parts[def->nBodyParts])
{
    ui32 counter = 0;
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(part->connectionId==parent->id)
        {
            parts[counter++] = part->id;
        }
    }
    return counter;
}

ui32
GetSubNodeBodyParts(CreatureDefinition *def, 
        BodyPartDefinition *parent,
        BodyPartDefinition *parts[def->nBodyParts])
{
    ui32 counter = 0;
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(part->connectionId==parent->id)
        {
            parts[counter++] = part;
        }
    }
    return counter;
}

// Also removes all subnodes
void
RemoveBodyPart(CreatureDefinition *def, ui32 id)
{
    BodyPartDefinition *parent = GetBodyPartById(def, id);
    ui32 parts[def->nBodyParts];
    ui32 nParts = GetSubNodeBodyPartsById(def, parent, parts);
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < nParts;
            bodyPartIdx++)
    {
        RemoveBodyPart(def, parts[bodyPartIdx]);
    }
    ArrayRemoveElement(def->bodyParts, sizeof(BodyPartDefinition), def->nBodyParts--, parent);
}


void
RecalculateSubNodeBodyParts(CreatureDefinition *def,
        BodyPartDefinition *parent)
{
    BodyPartDefinition *parts[def->nBodyParts];
    ui32 nSubNodes = GetSubNodeBodyParts(def, parent, parts);
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < nSubNodes;
            bodyPartIdx++)
    {
        BodyPartDefinition *subPart = parts[bodyPartIdx];
        Vec2 pivotPoint = GetBoxEdgePosition(parent->pos, 
                vec2(parent->width, parent->height),
                parent->angle,
                subPart->xEdge,
                subPart->yEdge,
                subPart->offset);
        r32 edgeAngle = atan2f(subPart->yEdge, subPart->xEdge);
        r32 totalAngle = NormalizeAngle(parent->angle + edgeAngle + subPart->localAngle);
        Vec2 center = v2_add(pivotPoint, v2_polar(totalAngle, subPart->width/2));

        subPart->pivotPoint = pivotPoint;
        subPart->pos = center;
        subPart->angle = totalAngle;
        RecalculateSubNodeBodyParts(def, subPart);
    }
    return;
}

#if 0
void
DefineGuy(CreatureDefinition *def)
{
    r32 size = 90;
    r32 headSize = 60;
    DefineBodyPart(def, vec2(0, 0), vec2(20, 2*size), 0);   // torso

    DefineBodyPart(def, vec2(size/2, -size), vec2(size, 20), 0);   // leg
    DefineBodyPart(def, vec2(3*size/2, -size), vec2(size, 20), 0);  
    DefineBodyPart(def, vec2(2*size, -size+size/4), vec2(20, size/2), 0);   // foot

    DefineBodyPart(def, vec2(-size/2, -size), vec2(size, 20), 0);   // leg
    DefineBodyPart(def, vec2(-3*size/2, -size), vec2(size, 20), 0);  
    DefineBodyPart(def, vec2(-2*size, -size+size/4), vec2(20, size/2), 0);   // foot

    // Arms
    DefineBodyPart(def, vec2(size/2, size), vec2(size, 20), 0);   
    DefineBodyPart(def, vec2(3*size/2, size), vec2(size, 20), 0);  
    DefineBodyPart(def, vec2(2*size+size/4, size), vec2(size/2, 20), 0);   

    DefineBodyPart(def, vec2(-size/2, size), vec2(size, 20), 0);   
    DefineBodyPart(def, vec2(-3*size/2, size), vec2(size, 20), 0);  
    DefineBodyPart(def, vec2(-2*size-size/4, size), vec2(size/2, 20), 0);   

    DefineBodyPart(def, vec2(0, size+10), vec2(20, 20), 0);   // neck
    DefineBodyPart(def, vec2(0, size+20+headSize/2), vec2(headSize, headSize), 0);   // head

    DefineRotaryMuscle(def, 0, 1, vec2(0, -size), -M_PI/2, 0.0);
    DefineRotaryMuscle(def, 1, 2, vec2(size, -size), -M_PI/2, 0.0);
    DefineRotaryMuscle(def, 2, 3, vec2(2*size, -size), -M_PI/2, 0.0);

    DefineRotaryMuscle(def, 0, 4, vec2(0, -size), 0, M_PI/2);
    DefineRotaryMuscle(def, 4, 5, vec2(-size, -size), 0, M_PI/2);
    DefineRotaryMuscle(def, 5, 6, vec2(-2*size, -size), 0, M_PI/2);

    // Arms
    DefineRotaryMuscle(def, 0, 7, vec2(0, size), -M_PI/2, M_PI/2);
    DefineRotaryMuscle(def, 7, 8, vec2(size, size), 0, M_PI);
    DefineRotaryMuscle(def, 8, 9, vec2(2*size, size), M_PI/2, -M_PI/2);

    DefineRotaryMuscle(def, 0, 10, vec2(0, size), -M_PI/2, M_PI/2);
    DefineRotaryMuscle(def, 10, 11, vec2(-size, size), -M_PI, 0);
    DefineRotaryMuscle(def, 11, 12, vec2(-2*size, size), M_PI/2, -M_PI/2);

    // Neck and head
    DefineRotaryMuscle(def, 0, 13, vec2(0, size), -0.1, 0.1);
    DefineRotaryMuscle(def, 13, 14, vec2(0, size+20), -0.1, 0.1);
}

void
DefineMilli(CreatureDefinition *def)
{
    DefineBodyPart(def, vec2(0, 0), vec2(40, 180), 0);

    DefineBodyPart(def, vec2(40, -80), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(40, -40), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(40, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(40, 40), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(40, 80), vec2(40, 10), 0);

    DefineRotaryMuscle(def, 0, 1, vec2(20, -80), -1, 1);
    DefineRotaryMuscle(def, 0, 2, vec2(20, -40), -1, 1);
    DefineRotaryMuscle(def, 0, 3, vec2(20, 0), -1, 1);
    DefineRotaryMuscle(def, 0, 4, vec2(20, 40), -1, 1);
    DefineRotaryMuscle(def, 0, 5, vec2(20, 80), -1, 1);

    DefineBodyPart(def, vec2(-40, -80), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-40, -40), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-40, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-40, 40), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-40, 80), vec2(40, 10), 0);

    DefineRotaryMuscle(def, 0, 6, vec2(-20, -80), -1, 1);
    DefineRotaryMuscle(def, 0, 7, vec2(-20, -40), -1, 1);
    DefineRotaryMuscle(def, 0, 8, vec2(-20, 0), -1, 1);
    DefineRotaryMuscle(def, 0, 9, vec2(-20, 40), -1, 1);
    DefineRotaryMuscle(def, 0, 10, vec2(-20, 80), -1, 1);
}

void
DefineBug(CreatureDefinition *def)
{
    DefineBodyPart(def, vec2(0, 0), vec2(40, 180), 0);

    DefineBodyPart(def, vec2(40, -80), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(40, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(40, 80), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(80, -80), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(80, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(80, 80), vec2(40, 10), 0);

    DefineRotaryMuscle(def, 0, 1, vec2(20, -80), -1, 1);
    DefineRotaryMuscle(def, 0, 2, vec2(20, 0), -1, 1);
    DefineRotaryMuscle(def, 0, 3, vec2(20, 80), -1, 1);
    DefineRotaryMuscle(def, 1, 4, vec2(60, -80), -1, 1);
    DefineRotaryMuscle(def, 2, 5, vec2(60, 0), -1, 1);
    DefineRotaryMuscle(def, 3, 6, vec2(60, 80), -1, 1);

    DefineBodyPart(def, vec2(-40, -80), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-40, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-40, 80), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-80, -80), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-80, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-80, 80), vec2(40, 10), 0);

    DefineRotaryMuscle(def, 0, 7, vec2(-20, -80), -1, 1);
    DefineRotaryMuscle(def, 0, 8, vec2(-20, 0), -1, 1);
    DefineRotaryMuscle(def, 0, 9, vec2(-20, 80), -1, 1);
    DefineRotaryMuscle(def, 7, 10, vec2(-60, -80), -1, 1);
    DefineRotaryMuscle(def, 8, 11, vec2(-60, 0), -1, 1);
    DefineRotaryMuscle(def, 9, 12, vec2(-60, 80), -1, 1);

}

void
DefineQuadruped(CreatureDefinition *def)
{
    DefineBodyPart(def, vec2(0, 0), vec2(40, 40), M_PI/4.0);

    DefineBodyPart(def, vec2(40, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(80, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(120, 0), vec2(40, 10), 0);

    DefineBodyPart(def, vec2(-40, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-80, 0), vec2(40, 10), 0);
    DefineBodyPart(def, vec2(-120, 0), vec2(40, 10), 0);

    DefineBodyPart(def, vec2(0, -40), vec2(10, 40), 0);
    DefineBodyPart(def, vec2(0, -80), vec2(10, 40), 0);
    DefineBodyPart(def, vec2(0, -120), vec2(10, 40), 0);

    DefineBodyPart(def, vec2(0, 40), vec2(10, 40), 0);
    DefineBodyPart(def, vec2(0, 80), vec2(10, 40), 0);
    DefineBodyPart(def, vec2(0, 120), vec2(10, 40), 0);

    DefineRotaryMuscle(def, 0, 1, vec2(20, 0), -1, 1);
    DefineRotaryMuscle(def, 1, 2, vec2(60, 0), -1, 1);
    DefineRotaryMuscle(def, 2, 3, vec2(100, 0), -1, 1);

    DefineRotaryMuscle(def, 0, 4, vec2(-20, 0), -1, 1);
    DefineRotaryMuscle(def, 4, 5, vec2(-60, 0), -1, 1);
    DefineRotaryMuscle(def, 5, 6, vec2(-100, 0), -1, 1);

    DefineRotaryMuscle(def, 0, 7, vec2(0, -20), -1, 1);
    DefineRotaryMuscle(def, 7, 8, vec2(0, -60), -1, 1);
    DefineRotaryMuscle(def, 8, 9, vec2(0, -100), -1, 1);

    DefineRotaryMuscle(def, 0, 10, vec2(0, 20), -1, 1);
    DefineRotaryMuscle(def, 10, 11, vec2(0, 60), -1, 1);
    DefineRotaryMuscle(def, 11, 12, vec2(0, 100), -1, 1);
}

#endif


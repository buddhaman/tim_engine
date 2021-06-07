
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
    return NormalizeAngle(absAngle - parent->angle - edgeAngle);
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
        subPart->degree = parent->degree+1;
        Vec2 pivotPoint = GetBoxEdgePosition(parent->pos, 
                vec2(parent->width, parent->height),
                parent->angle,
                subPart->xEdge,
                subPart->yEdge,
                subPart->offset);
        r32 edgeAngle = atan2f(subPart->yEdge, subPart->xEdge);
        r32 totalAngle = NormalizeAngle(parent->angle + edgeAngle) + subPart->localAngle;
        Vec2 center = v2_add(pivotPoint, v2_polar(totalAngle, subPart->width/2));

        subPart->pivotPoint = pivotPoint;
        subPart->pos = center;
        subPart->angle = totalAngle;
        RecalculateSubNodeBodyParts(def, subPart);
    }
}

int
DegreeCompareFunction(const void *p0, const void *p1)
{
    const BodyPartDefinition **part0 = (const BodyPartDefinition **)p0;
    const BodyPartDefinition **part1 = (const BodyPartDefinition **)p1;
    int result = part1[0]->degree;
    result-=part0[0]->degree;
    return result;
}

void
RecalculateBodyPartDrawOrder(CreatureDefinition *def)
{
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts; 
            bodyPartIdx++)
    {
        def->drawOrder[bodyPartIdx] = def->bodyParts+bodyPartIdx;
    }
    qsort(def->drawOrder, def->nBodyParts, sizeof(BodyPartDefinition*), DegreeCompareFunction);
}

void
AssignBrainIO(CreatureDefinition *def)
{
    ui32 atInputIdx = def->nInternalClocks;
    ui32 atOutputIdx = 0;
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *bodyPartDef = def->bodyParts+bodyPartIdx;
        //Inputs
        if(bodyPartDef->hasAbsoluteXPositionInput) bodyPartDef->absoluteXPositionInputIdx = atInputIdx++;
        if(bodyPartDef->hasAbsoluteYPositionInput) bodyPartDef->absoluteYPositionInputIdx = atInputIdx++;

        // Outputs
        if(bodyPartDef->hasDragOutput) bodyPartDef->dragOutputIdx = atOutputIdx++;
        if(bodyPartDef->hasRotaryMuscleOutput) bodyPartDef->rotaryMuscleOutputIdx = atOutputIdx++;
    }
    
    def->nInputs = atInputIdx;
    def->nOutputs = atOutputIdx;
    def->geneSize = GetMinimalGatedUnitGeneSize(def->nInputs, def->nOutputs, def->nHidden);
}

void
GenerateRandomName(char *name, ui32 maxLength)
{
    ui32 nSyls = 0;
    char *syls[64];
    syls[nSyls++] = "ha"; 
    syls[nSyls++] = "hu"; 
    syls[nSyls++] = "ho";
    syls[nSyls++] = "la";
    syls[nSyls++] = "pe";
    syls[nSyls++] = "nis";
    syls[nSyls++] = "yo";
    syls[nSyls++] = "ji";
    syls[nSyls++] = "bru";
    syls[nSyls++] = "hm";
    ui32 atChar = 0;
    for(ui32 atSyl = 0; 
            atSyl < 3;
            atSyl++)
    {
        char *syl = syls[RandomUI32(0, nSyls)];
        ui32 len = strlen(syl);
        if(atChar+len < maxLength-3)
        {
            strcpy(name+atChar, syl);
            atChar+=strlen(syl);
        }
        else
        {
            break;
        }
    }
    char number[10];
    sprintf(number, "%03u", RandomUI32(0, 1000));
    strcpy(name+atChar, number);
}

void
GeneratePathNames(CreatureDefinition *def, char *dataPath, char *texturePath)
{
    strcpy(dataPath, CREATURE_FOLDER_NAME);
    strcat(dataPath, "/");
    strcat(dataPath, def->name);
    strcat(dataPath, ".crdf");

    strcpy(texturePath, CREATURE_FOLDER_NAME);
    strcat(texturePath, "/");
    strcat(texturePath, def->name);
    strcat(texturePath, ".png");
}


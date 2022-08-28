
U32
GetIndexOfBodyPart(CreatureDefinition *def, U32 id)
{
    for(U32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(part->id == id) return bodyPartIdx;
    }
    return -1;
}

BodyPartDefinition *
GetBodyPartById(CreatureDefinition *def, U32 id)
{
    for(U32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(part->id == id) return part;
    }
    return NULL;
}

R32
GetLocalAngleFromAbsoluteAngle(CreatureDefinition *def,
        BodyPartDefinition *part,
        R32 absAngle)
{
    R32 edgeAngle = atan2f(part->yEdge, part->xEdge);
    BodyPartDefinition *parent = GetBodyPartById(def, part->connectionId);
    return NormalizeAngle(absAngle - parent->angle - edgeAngle);
}

R32 
GetAbsoluteEdgeAngle(BodyPartDefinition *part, int xEdge, int yEdge)
{
    return NormalizeAngle(atan2f(yEdge, xEdge)+part->angle);
}

internal inline B32
BodyPartTexturePoint2Intersect(BodyPartDefinition *part, R32 textureOverhang, Vec2 point)
{
    return OrientedBoxPoint2Intersect(part->pos,
            V2(part->width+textureOverhang*2, part->height+textureOverhang*2), part->angle, point);
}

U32
GetSubNodeBodyPartsById(CreatureDefinition *def, 
        BodyPartDefinition *parent,
        U32 parts[def->nBodyParts])
{
    U32 counter = 0;
    for(U32 bodyPartIdx = 0;
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

U32
GetSubNodeBodyParts(CreatureDefinition *def, 
        BodyPartDefinition *parent,
        BodyPartDefinition *parts[def->nBodyParts])
{
    U32 counter = 0;
    for(U32 bodyPartIdx = 0;
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
    U32 nSubNodes = GetSubNodeBodyParts(def, parent, parts);
    for(U32 bodyPartIdx = 0;
            bodyPartIdx < nSubNodes;
            bodyPartIdx++)
    {
        BodyPartDefinition *subPart = parts[bodyPartIdx];
        subPart->degree = parent->degree+1;
        Vec2 pivotPoint = GetBoxEdgePosition(parent->pos, 
                V2(parent->width, parent->height),
                parent->angle,
                subPart->xEdge,
                subPart->yEdge,
                subPart->offset);
        R32 edgeAngle = atan2f(subPart->yEdge, subPart->xEdge);
        R32 totalAngle = NormalizeAngle(parent->angle + edgeAngle) + subPart->localAngle;
        Vec2 center = V2Add(pivotPoint, V2Polar(totalAngle, subPart->width/2));

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
    for(U32 bodyPartIdx = 0;
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
    U32 atInputIdx = def->nInternalClocks;
    U32 atOutputIdx = 0;
    for(U32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *bodyPartDef = def->bodyParts+bodyPartIdx;
        //Inputs
        if(bodyPartDef->hasAbsoluteXPositionInput) bodyPartDef->absoluteXPositionInputIdx = atInputIdx++;
        if(bodyPartDef->hasAbsoluteYPositionInput) bodyPartDef->absoluteYPositionInputIdx = atInputIdx++;
        if(bodyPartDef->hasAngleTowardsTargetInput) bodyPartDef->angleTowardsTargetInputIdx = atInputIdx++;
        if(bodyPartDef->hasAbsoluteAngleInput) bodyPartDef->absoluteAngleInputIdx = atInputIdx++;

        // Outputs
        if(bodyPartDef->hasDragOutput) bodyPartDef->dragOutputIdx = atOutputIdx++;
        if(bodyPartDef->hasRotaryMuscleOutput) bodyPartDef->rotaryMuscleOutputIdx = atOutputIdx++;
    }
    
    def->nInputs = atInputIdx;
    def->nOutputs = atOutputIdx;
    def->geneSize = GetMinimalGatedUnitGeneSize(def->nInputs, def->nOutputs, def->nHidden);
}

void
GenerateRandomName(char *name, U32 maxLength)
{
    U32 nSyls = 0;
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
    U32 atChar = 0;
    for(U32 atSyl = 0; 
            atSyl < 3;
            atSyl++)
    {
        char *syl = syls[RandomUI32(0, nSyls)];
        U32 len = strlen(syl);
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


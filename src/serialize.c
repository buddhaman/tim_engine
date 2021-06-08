
void
BeginSerializing(Serializer *serializer, char *path, b32 write)
{
    serializer->file = fopen(path, write ? "wb" : "rb");
    if(serializer->file)
    {
        serializer->isWriting = write;
        if(serializer->isWriting)
        {
            serializer->dataVersion = LATEST_VERSION;
            fwrite(&serializer->dataVersion, sizeof(ui32), 1, serializer->file);
        }
        else
        {
            fread(&serializer->dataVersion, sizeof(ui32), 1, serializer->file);
        }
    }
    else
    {
        DebugOut("ERROR: File not found: %s", path);
    }
}

void
EndSerializing(Serializer *serializer)
{
    fclose(serializer->file);
}

#define SerializePrimitiveTypeFunction(name, type) void name(Serializer *serializer, type *value)\
{\
    if(serializer->isWriting)\
    {\
        fwrite(value, sizeof(type), 1, serializer->file);\
    }\
    else\
    {\
        fread(value, sizeof(type), 1, serializer->file);\
    }\
}\

SerializePrimitiveTypeFunction(SerializeUI32, ui32)
SerializePrimitiveTypeFunction(SerializeR32, r32);
SerializePrimitiveTypeFunction(SerializeVec2, Vec2);
SerializePrimitiveTypeFunction(SerializeVec3, Vec3);
SerializePrimitiveTypeFunction(SerializeI32, i32);
SerializePrimitiveTypeFunction(SerializeB32, b32);

void
SerializeB32Array(Serializer *serializer, b32 *array, size_t len)
{
    if(serializer->isWriting)
    {
        fwrite(array, sizeof(b32), len, serializer->file);
    }
    else
    {
        fread(array, sizeof(b32), len, serializer->file);
    }
}

void
SerializeString(Serializer *serializer, char *string, size_t len)
{
    if(serializer->isWriting)
    {
        fwrite(string, sizeof(char), len, serializer->file);
    }
    else
    {
        fread(string, sizeof(char), len, serializer->file);
    }
}

void SerializeBodyPartDefinition(Serializer *serializer, BodyPartDefinition *partDef)
{
    SerializeUI32(serializer, &partDef->id);
    SerializeVec2(serializer, &partDef->pos);
    SerializeR32(serializer, &partDef->width);
    SerializeR32(serializer, &partDef->height);
    SerializeR32(serializer, &partDef->angle);
    SerializeR32(serializer, &partDef->localAngle);

    SerializeUI32(serializer, &partDef->connectionId);
    SerializeI32(serializer, &partDef->xEdge);
    SerializeI32(serializer, &partDef->yEdge);
    SerializeR32(serializer, &partDef->offset);
    SerializeVec2(serializer, &partDef->pivotPoint);
    SerializeR32(serializer, &partDef->minAngle);
    SerializeR32(serializer, &partDef->maxAngle);

    SerializeB32(serializer, &partDef->hasDragOutput);
    SerializeB32(serializer, &partDef->hasRotaryMuscleOutput);
    SerializeB32(serializer, &partDef->hasAbsoluteXPositionInput);
    SerializeB32(serializer, &partDef->hasAbsoluteYPositionInput);

    SerializeUI32(serializer, &partDef->absoluteXPositionInputIdx);
    SerializeUI32(serializer, &partDef->absoluteYPositionInputIdx);

    SerializeUI32(serializer, &partDef->dragOutputIdx);
    SerializeUI32(serializer, &partDef->rotaryMuscleOutputIdx);

    SerializeR32(serializer, &partDef->rotaryMuscleStrength);

    SerializeVec2(serializer, &partDef->uvPos);
    SerializeVec2(serializer, &partDef->uvDims);

    SerializeI32(serializer, &partDef->texGridX);
    SerializeI32(serializer, &partDef->texGridY);
    SerializeR32(serializer, &partDef->texScale);
}

void
SerializeCreatureDefinition(Serializer *serializer, CreatureDefinition *def)
{
    SerializeUI32(serializer, &def->nBodyParts);
    SerializeUI32(serializer, &def->nInputs);
    SerializeUI32(serializer, &def->nOutputs);
    SerializeUI32(serializer, &def->nHidden);
    SerializeUI32(serializer, &def->geneSize);
    SerializeUI32(serializer, &def->nInternalClocks);
    SerializeR32(serializer, &def->textureOverhang);
    SerializeB32(serializer, &def->drawSolidColor);
    SerializeVec3(serializer, &def->solidColor);

    SerializeUI32(serializer, &def->creatureTextureGridDivs);
    SerializeB32Array(serializer, def->isTextureSquareOccupied, 16);

    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        SerializeBodyPartDefinition(serializer, def->bodyParts+bodyPartIdx);
    }
    
    SerializeString(serializer, def->name, MAX_CREATURE_NAME_LENGTH);
}


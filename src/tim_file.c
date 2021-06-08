

b32
StringStartsWith(char *string, char *prefix)
{
    size_t prefixLength = strlen(prefix);
    return strncmp(string, prefix, prefixLength)==0;
}

b32
StringEndsWith(char *string, char *suffix)
{
    size_t suffixLength = strlen(suffix);
    size_t stringLength = strlen(string);
    if(stringLength >= suffixLength)
    {
        return strcmp(string+stringLength-suffixLength, suffix)==0;
    }
    else
    {
        return 0;
    }
}

char *
FindWithPrefix(int nStrings, char *strings[nStrings], char *prefix)
{
    for(ui32 stringIdx = 0;
            stringIdx < nStrings;
            stringIdx++)
    {
        char *string = strings[stringIdx];
        if(StringStartsWith(string, prefix))
        {
            return string;
        }
    }
    return NULL;
}

void
PairCreatureFiles(int maxCreatures, 
        CreatureDefinitionFile creatureDefinitionFiles[maxCreatures],
        int nCreatureDataFiles,
        char *creatureDataFiles[nCreatureDataFiles],
        int nCreatureTextureFiles,
        char *creatureTextureFiles[nCreatureTextureFiles])
{
    ui32 matchCounter = 0;
    for(ui32 creatureIdx = 0;
            creatureIdx < nCreatureDataFiles;
            creatureIdx++)
    {
        char *creatureDataPath = creatureDataFiles[creatureIdx];
        size_t dataPathLength = strlen(creatureDataPath);
        char *suffix = ".crdf";
        size_t suffixLength = strlen(suffix);
        char prefix[dataPathLength-suffixLength+1];
        memcpy(prefix, creatureDataPath, sizeof(char)*(dataPathLength-suffixLength));
        prefix[dataPathLength-suffixLength] = 0;
        char *texturePath = FindWithPrefix(nCreatureTextureFiles, creatureTextureFiles, prefix);
        if(texturePath)
        {
            CreatureDefinitionFile *result = creatureDefinitionFiles+matchCounter++;
            strcpy(result->dataPath, creatureDataPath);
            strcpy(result->texturePath, texturePath);
            if(matchCounter > maxCreatures)
            {
                break;
            }
        }
    }
}


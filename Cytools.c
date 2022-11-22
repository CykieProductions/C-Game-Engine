#include <stdio.h>
#include <string.h>
//#include <sys/stat.h>
#include "Cytools.h"

bool CompareStrings(char* inp1, char* inp2)
{
    size_t length = strlen(inp1);
    if (strlen(inp2) > length)
        length = strlen(inp2);

    for (size_t i = 0; i < length; i++)
    {
        //because of strlen, this loop can only find the NULL in a shorter string (meaning they aren't equal)
        if (inp1[i] == '\0' || inp2[i] == '\0')
            return false;
        if (inp1[i] != inp2[i])
            return false;
    }
    
    return true;
}

int SetString(char* finalStr, char* input, size_t sizeOfFinal, size_t sizeOfInput)
{
    for (size_t i = 0; i < sizeOfFinal; i++)
    {
        /*if (i == 62)
            printf("T");*/

        if (i < sizeOfInput && i < sizeOfFinal - 1)
            finalStr[i] = input[i];
        else
            finalStr[i] = '\0';//if done copying or one space left, then fill with NULL
        //printf(finalStr);
    }

    return 0;
}
int AppendString(char* finalStr, char* input, size_t sizeOfFinal, size_t sizeOfInput)
{
    size_t ogLength = strlen(finalStr);
    for (size_t i = strlen(finalStr); i < sizeOfFinal; i++)//Start on the Null terminator
    {
        /*if (i == 62)
            printf("T");*/

        if (i - ogLength < sizeOfInput && i < sizeOfFinal - 1)
            finalStr[i] = input[i - ogLength];
        else
            finalStr[i] = '\0';//if done copying or one space left, then fill with NULL
        //printf(finalStr);
    }

    return 0;
}

//https://stackoverflow.com/questions/2218290/concatenate-char-array-in-c
//based on mctylr's post
int ConcatStrings(char finalString[], size_t sizeOfFinal, char input1[], char input2[])
{
    rsize_t size;
    size = sizeOfFinal;
    size = strlen(input1) + 1;

    if (sizeOfFinal < strlen(input1) + 1) { /* +1 is for null character */
        //fprintf(stderr, "Name '%s' is too long\n", input1);
        return 0;//Fail
    }
    //strncpy_s(finalString, input1, sizeOfFinal, strlen(input1));
    SetString(finalString, input1, sizeOfFinal, strlen(input1) + 1);

    if (sizeOfFinal < (strlen(finalString) + strlen(input2) + 1)) {
        //fprintf(stderr, "Final size of finalString is too long!\n");
        return 0;//Fail
    }
    AppendString(finalString, input2, sizeOfFinal, strlen(input2) + 1);
    //printf("Filename is %s\n", finalString);

    return 1;//Success
}


int ClampInt(int input, int min, int max)
{
    if (min > max)
        min = max;

    if (input >= max)
        input = max;
    else if (input <= min)
        input = min;

    return input;
}
float ClampFloat(float input, float min, float max)
{
    if (min > max)
    {
        float tmp = min;
        min = max;
        max = tmp;
    }

    if (input >= max)
        input = max;
    else if (input <= min)
        input = min;

    return input;
}
float SnapFloatTo(float input, float min, float max)
{
    if (min > max)
    {
        float tmp = min;
        min = max;
        max = tmp;
    }

    int diff = max - min;

    if (input + diff >= max)
        input = max;
    else
        input = min;

    return input;
}
int WrapInt(int input, int min, int max)
{
    if (min > max)
        min = max;

    if (input >= max)
        input = min;//overflow
    else if (input <= min)
        input = max;//underflow

    return input;
}

float WrapFloat(float input, float min, float max)
{
    if (min > max)
        min = max;

    if (input > max)
    {
        float extra = input - max;
        input = min + extra;//overflow
    }
    else if (input < min)
    {
        float extra = min - input;
        input = max - extra;//underflow
    }

    return input;
}

void FillArrayWithValue(void* array[],size_t sizeOfArray, void* value, size_t valSize)
{
    memset(array, value, sizeOfArray);// * valSize);
    //for (size_t i = 0; i < length; i++)
    //{
    //}
}


BOOL IsFileValid(char* path)
{
    FILE file = { 0 };
    bool result = false;

    if (fopen_s(&file, path, "r") != 0)//Read file or folder
        goto Exit;
    if (&file == NULL)
        goto Exit;

    result = true;

Exit:
    //fclose(&file);
    return result;
}

/*BOOL IsDirectoryValid(char* path)
{
    struct stat stats;

    stat(path, &stats);

    // Check for file existence
    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}*/
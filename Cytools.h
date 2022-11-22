#pragma once

#include <stdlib.h>
#include <math.h>

//I got fed up with strcpy, strncpy, strncpy_s, strcat, strncat, and whatever else!
//I made my own stuff that works through for loops

typedef unsigned int uint;

typedef int BOOL;
typedef _Bool bool;
//typedef _Bool boolean;
//typedef unsigned short bool
//#define bool _Bool
#define FALSE 0
#define false 0
#define TRUE 1
#define true 1

//void IterateLoop(_Inout_ size_t* i, _In_ int factor, _In_ bool isPositive);
bool CompareStrings(char* inp1, char* inp2);
int SetString(char* finalStr, char* input, size_t sizeOfFinal, size_t sizeOfInput);
int AppendString(char* finalStr, char* input, size_t sizeOfFinal, size_t sizeOfInput);
int ConcatStrings(char finalString[], size_t sizeOfFinal, char input1[], char input2[]);

//Returns the clamped int
int ClampInt(int input, int min, int max);
float ClampFloat(float input, float min, float max);
float SnapFloatTo(float input, float min, float max);
int WrapInt(int input, int min, int max);
float WrapFloat(float input, float min, float max);

void FillArrayWithValue(void* array[], size_t sizeOfArray, void* value, size_t valSize);
BOOL IsFileValid(char* path);
#pragma once
#include <windows.h>
#include <stdint.h>
#include <Psapi.h>
#include <emmintrin.h>

#include "Vectors.h"
#include "Cytools.h"

//OpenGL 1.1 (Like it's 1996!)
#ifdef OPENGL
#include <gl/GL.h>
#pragma comment(lib, "OpenGL32.lib")
#endif // OPENGL

#define RES_WIDTH	384
#define RES_HEIGHT	240//216
#define RES_ASPECT_RATIO ((float)RES_WIDTH / RES_HEIGHT)
#define GAME_BPP		32
#define FRAME_MEMORY_SIZE	(RES_WIDTH * RES_HEIGHT * (GAME_BPP / 8))

#define MIC_TO_SEC 0.000001
#define SEC_TO_MIC 1000000

//RECT gWindowSize;

// STRUCTS //

typedef struct GAMEBITMAP
{
	void* Memory;
	BITMAPINFO bitmapInfo;
	int pivotX;
	int pivotY;

} GAMEBITMAP;

typedef struct GameDebugData
{
	BOOL showDebugText;
	BOOL windowHasFocus;
	bool isInEditMode;

	char defaultDataPath[128];

	DWORD handleCount;
	PROCESS_MEMORY_COUNTERS_EX memInfo;

	SYSTEM_INFO systemInfo;
	long curSystemTime;
	long prevSystemTime;

	long curUserCPUTime;
	long curKernelCPUTime;
	
	long prevUserCPUTime;
	long prevKernelCPUTime;

	float cpuPercent;

} GameDebugData;

typedef struct GameTimeData
{
	uint64_t totalFramesRendered;

	double rawFPSAverage;
	double realFPSAverage;
	double deltaTime;

	int framesBeforeAvgCalc;
	float targetFPS;
	uint64_t targetMicroseconds;

	uint64_t perfFrequency;
	uint64_t elapsedMicroSeconds;

	long maxTimerRes;
	long minTimerRes;
	long curTimerRes;

} GameTimeData;

typedef struct Color32
{
	//This order is important! RGBA is BGRA in memory
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t alpha;
	//uint8_t == unsigned char
} Color32;
typedef union ColorF {
	struct
	{
		float blue;
		float green;
		float red;
		float alpha;
	};
	Vector4 BGRA;
} ColorF; 
//https://docs.geoserver.org/main/en/user/styling/sld/extensions/composite-blend/modes.html#:~:text=Alpha%20compositing%20modes%20%C2%B6%201%20copy%20%C2%B6%20Only,%C2%B6%20...%208%20destination-out%20%C2%B6%20...%20More%20items
typedef enum AC_BlendModes
{
	AC_SrcOver
} AC_BlendModes;

//

//Globals//
extern GAMEBITMAP gBackBuffer;
extern GameTimeData gTime;
extern GameDebugData gDebug;
//

//Functions//
BOOL InitBackBuffer(void);
void InitTimeData(void);

int GetStartingPixel(int width, int height, int x, int y);
void RenderFrame(void);

void ClearScreen(__m128i color);

DWORD Load32BppBitmapFromFile(_Inout_ GAMEBITMAP* bitmap, _In_ char* filePath);
void LoadSpriteFromSheet(GAMEBITMAP* dest, GAMEBITMAP* sheet, int offsetX, int offsetY, int width, int height,
	int pivX, int pivY, bool flipped);

//void Blit32BppBitmapToBuffer(_Inout_ GAMEBITMAP* bitmap, _In_ unsigned int x, _In_ unsigned int y);

Vector2LONG GetResolution(GAMEBITMAP* bitmap);
void SetResolution(GAMEBITMAP* bitmap, long x, long y);

ColorF Color32ToColorF(Color32* color);
void AlphaBlendColor32(_Inout_ Color32* over_, _In_ const Color32* og_, _In_ const AC_BlendModes MODE);
//

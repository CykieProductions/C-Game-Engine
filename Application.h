#pragma once
#include <stdint.h>
#include <Windows.h>
#include <Psapi.h>
#include "DriverSystem.h"
#include "Cytools.h"

//#define OPENGL//Not yet?

//OpenGL 1.1 (Like it's 1996!)
#ifdef OPENGL
#include <gl/GL.h>
#pragma comment(lib, "OpenGL32.lib")
#endif // OPENGL

typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG minResolution, OUT PULONG maxResolution, OUT PULONG curResolution);
_NtQueryTimerResolution NtQueryTimerResolution;


#define VK_0KEY	0x30
#define VK_1KEY 0x31
#define VK_2KEY 0x32
#define VK_3KEY	0x33
#define VK_4KEY 0x34
#define VK_5KEY	0x35
#define VK_6KEY 0x36
#define VK_7KEY	0x37
#define VK_8KEY 0x38
#define VK_9KEY	0x39

typedef struct ButtonInput
{
	bool pressed;
	bool held;
	bool released;

} ButtonInput;

typedef struct KeyboardInput
{
	ButtonInput leftKey;
	ButtonInput rightKey;
	ButtonInput upKey;
	ButtonInput downKey;

	ButtonInput qKey;
	ButtonInput wKey;
	ButtonInput eKey;
	ButtonInput rKey;
	ButtonInput tKey;
	ButtonInput yKey;
	ButtonInput uKey;
	ButtonInput iKey;
	ButtonInput oKey;
	ButtonInput pKey;
	ButtonInput aKey;
	ButtonInput sKey;
	ButtonInput dKey;
	ButtonInput fKey;
	ButtonInput gKey;
	ButtonInput hKey;
	ButtonInput jKey;
	ButtonInput kKey;
	ButtonInput lKey;
	ButtonInput zKey;
	ButtonInput xKey;
	ButtonInput cKey;
	ButtonInput vKey;
	ButtonInput bKey;
	ButtonInput nKey;
	ButtonInput mKey;

	//Number
	ButtonInput numRow1;
	ButtonInput numRow2;
	ButtonInput numRow3;
	ButtonInput numRow4;
	ButtonInput numRow5;
	ButtonInput numRow6;
	ButtonInput numRow7;
	ButtonInput numRow8;
	ButtonInput numRow9;
	ButtonInput numRow0;

	ButtonInput numPad1;
	ButtonInput numPad2;
	ButtonInput numPad3;
	ButtonInput numPad4;
	ButtonInput numPad5;
	ButtonInput numPad6;
	ButtonInput numPad7;
	ButtonInput numPad8;
	ButtonInput numPad9;
	ButtonInput numPad0;

	ButtonInput number1;
	ButtonInput number2;
	ButtonInput number3;
	ButtonInput number4;
	ButtonInput number5;
	ButtonInput number6;
	ButtonInput number7;
	ButtonInput number8;
	ButtonInput number9;
	ButtonInput number0;

	//Special

	ButtonInput spaceBar;
	ButtonInput leftShiftKey;
	ButtonInput rightShiftKey;
	ButtonInput shiftKey;
	ButtonInput escapeKey;
	ButtonInput tabKey;
	ButtonInput enterKey;

} KeyboardInput;
typedef struct MouseInput
{
	CURSORINFO cursorInfo;
	Vector2INT pixelPosition;

	short deltaScroll;
	short scrollDir;

	Vector2 deltaMove;
	Vector2 moveDir;

	bool leftButtonPressed;
	bool leftButtonHeld;
	bool leftButtonReleased;

	bool middleButtonPressed;
	bool middleButtonHeld;
	bool middleButtonReleased;

	bool rightButtonPressed;
	bool rightButtonHeld;
	bool rightButtonReleased;

} MouseInput;

typedef struct APP_INPUT
{
	MouseInput mouse;
	KeyboardInput keyboard;
} APP_INPUT;

//Globals//

extern char GAME_NAME[64];
extern wchar_t WGAME_NAME[sizeof(GAME_NAME)];

extern bool gIsPlaying;
extern HWND gMainWindow;
extern MONITORINFO gMonitorInfo;
extern int32_t gMonitorWidth;
extern int32_t gMonitorHeight;
extern APP_INPUT Input;

extern Entity player;
//


//SETUP//
#ifdef OPENGL
DWORD InitOpenGL(void);
#endif

DWORD CreateMainWindow(_In_ HINSTANCE hInstance);
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL IsGameAlreadyRunning(void);
//ASCII to Unicode
//Returns a wide string pointer
LPCWSTR GameNameToWide(void);

void ProcessPlayerInput(void);

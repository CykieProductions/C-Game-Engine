#include <stdio.h>
#include "Renderer.h"
#include "Application.h"
#include "DriverSystem.h"
#include "Cytools.h"

//Externs
GAMEBITMAP gBackBuffer = { 0 };
GameTimeData gTime = { 0 };
GameDebugData gDebug = { 0 };
//RECT gWindowSize = { 0 };

int GetStartingPixel(int width, int height, int x, int y)
{
	return ((width * height) - width) - ((width * y) + x);
}

BOOL InitBackBuffer(void)
{
	gBackBuffer.bitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.bitmapInfo.bmiHeader);
	SetResolution(&gBackBuffer, RES_WIDTH, RES_HEIGHT);
	gBackBuffer.bitmapInfo.bmiHeader.biBitCount = GAME_BPP;
	gBackBuffer.bitmapInfo.bmiHeader.biCompression = BI_RGB;
	gBackBuffer.bitmapInfo.bmiHeader.biPlanes = 1;

	if ((gBackBuffer.Memory = VirtualAlloc(NULL, FRAME_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) == NULL)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void InitTimeData(void)
{
	gTime.framesBeforeAvgCalc = 100;
	gTime.totalFramesRendered = 0;
	gTime.targetMicroseconds = 16667;
	gTime.targetFPS = 60;

	//FillArrayWithValue(gGameManager.allTransforms, sizeof(gGameManager.allTransforms), NULL, sizeof(NULL));
	//FillArrayWithValue(gGameManager.allSpriteRenderers, sizeof(gGameManager.allSpriteRenderers), NULL, sizeof(NULL));
	//FillArrayWithValue(gGameManager.allAnimators, sizeof(gGameManager.allAnimators), NULL, sizeof(NULL));
}

__forceinline void ClearScreen(__m128i color)//Inline - copy the copy in place of the function
{
	for (int x = 0; x < RES_WIDTH * RES_HEIGHT; x += 4)
	{
		_mm_store_si128((Color32*)gBackBuffer.Memory + x, color);
	}
}

Color32 PerPixel(Vector2 coord)
{
	Color32 color = { 0, (coord.y * 0.5f - 0.5f) * 255, (coord.x * 0.5f - 0.5f) * 255, 255 };
	//return color;

	//Transform* trans_ = (Transform*)GetComponent(DSID_Transform, &player);

	Vector2 rayOrigin = { 0 };//{ trans_->positionX, trans_->positionY };
	Vector2 rayTarget = { coord.x * RES_ASPECT_RATIO, coord.y };

	//if (coord.x > 0)
		//return (Pixel32) { 0 };

	Vector2 circleOrigin = { Input.mouse.pixelPosition.x, Input.mouse.pixelPosition.y }; //{ 50, 30 };
	static float test = 0;
	test -= Input.mouse.rightButtonHeld / 15000.0f;
	test += Input.mouse.leftButtonHeld / 15000.0f;
	float radius = 5.0f + test;

	//x (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	/*Standard Form - Circle:
		//Initial
			(X - Hx)^2 + (Y - Hy)^2 - r^2 = 0
		//Substitute in ray formula
			((Ax + Bxt) - Hx)^2 + ((Ay + Byt) - Hy)^2 - r^2 = 0
		//FOIL
			(Ax^2 - 2AxHx + &Bx^2*t^2 + @2AxBx*t + Hx^2 - @2BxHx*t) + (Ay^2 - 2AyHy + &By^2*t^2 + @2AyBy*t + Hy^2 - @2ByHy*t) - r^2
		//Quadratic equation in terms of 't'
			(Bx^2 + By^2)t^2 + (2AxBx + 2AyBy - 2BxHx - 2ByHy)t + (Ax^2 - 2AxHx + Hx^2 + Ay^2 - 2AyHy + Hy^2 - r^2)
	Where:
		A = ray origin
		B = ray target
		H = circle origin
		r = radius
		t = hit distance
	*/

	float a = Vec2Dot(rayTarget, rayTarget);
	float b = 2.0f * Vec2Dot(rayOrigin, rayTarget) - 2.0f * Vec2Dot(rayTarget, circleOrigin);
	float c = pow(rayOrigin.x, 2) - (2 * rayOrigin.x * circleOrigin.x) + pow(circleOrigin.x, 2) \
		+ pow(rayOrigin.y, 2) - (2 * rayOrigin.y * circleOrigin.y) + pow(circleOrigin.y, 2) - pow(radius, 2);
	/*float a = Vec2Dot(rayTarget, rayTarget);
	float b = 2.0f * Vec2Dot(rayOrigin, rayTarget);
	float c = Vec2Dot(rayOrigin, rayOrigin) - pow(radius, 2);*/

	/*Quadratic Formula:
		(-b +- sqrt(discriminant)) / (2 * a)
	Where:
		discriminant = b^2 - 4ac
	*/
	float discrim = pow(b, 2) - 4 * a * c;

	//On hit
	if (discrim >= 0)
	{
		//2 solutions
		float distances[2] = {
			(-b + sqrt(discrim)) / (2 * a),
			(-b - sqrt(discrim)) / (2 * a),
		};

		for (short i = 0; i < 2; i++)
		{
			Vector2 hitPos = (Vector2){ 
				rayOrigin.x + rayTarget.x * distances[i],
				rayOrigin.y + rayTarget.y * distances[i]
			};
			Vector2 normal = (Vector2){
				hitPos.x - circleOrigin.x,
				hitPos.y - circleOrigin.y
			};

			/*static float timer = 0;
			static int tDir = 1;
			timer += 0.000001f * 50 * tDir;
			if (timer > radius + 1)
				tDir = -1;
			else if (timer < -radius - 1)
				tDir = 1;*/
			//timer = WrapFloat(timer, -radius, radius);

			Vec2Normalize(&normal);
			/*Vector2F lightDir = {1 , 1};
			Vec2Normalize(&lightDir);*/

			float lightLevel = 1;//fmaxf(Vec2Dot(normal, Vec2MultiplyF(&lightDir, -1)), 0.0f);

			color = (Color32){
				//(normal.x * 0.5f + 0.5f) * 255,
				WrapFloat(lightLevel * 0.5f + 0.5f, 0, 1) * 255,
				WrapFloat(lightLevel * 0.5f + 0.5f, 0, 1) * 100,
				WrapFloat(lightLevel * 0.5f + 0.5f, 0, 1) * 100,
				255
			};
		}

		if (Vec2Distance(circleOrigin, coord) < radius)
			return color;
		else
			return (Color32){100, 0, 0, 50};
	}
	
	return (Color32){ 0 };
}

Color32 _PerPixel3D(Vector2 coord)
{
	Color32 color = { 0, (coord.y * 0.5f - 0.5f) * 255, (coord.x * 0.5f - 0.5f) * 255, 255 };

	Vector3 rayOrigin	= { 0, 0, 20 };
	Vector3 rayDir		= { coord.x * RES_ASPECT_RATIO, coord.y, -1 };

	Vector3 sphereOrigin = { Input.mouse.pixelPosition.x - (RES_WIDTH / 2), (RES_HEIGHT - Input.mouse.pixelPosition.y - (RES_HEIGHT / 2)), 0 };
	//Vector3F sphereOrigin = { 0, 0, 0 };
	float radius = 5.0f;

	/*Standard Form - Circle:
		(bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	Where:
		a = ray origin
		b = ray direction
		r = radius
		t = hit distance
	*/

	//float a = Vec3Dot(rayDir, rayDir);
	//float b = 2.0f * Vec3Dot(rayOrigin, rayDir);
	//float c = Vec3Dot(rayOrigin, rayOrigin) - pow(radius, 2);

	float a = Vec3Dot(rayDir, rayDir);
	float b = 2.0f * Vec3Dot(rayOrigin, rayDir) - 2.0f * Vec3Dot(rayDir, sphereOrigin);
	float c = pow(rayOrigin.x, 2) - (2 * rayOrigin.x * sphereOrigin.x) + pow(sphereOrigin.x, 2) \
		+ pow(rayOrigin.y, 2) - (2 * rayOrigin.y * sphereOrigin.y) + pow(sphereOrigin.y, 2) \
		+ pow(rayOrigin.z, 2) - (2 * rayOrigin.z * sphereOrigin.z) + pow(sphereOrigin.z, 2) \
		- pow(radius, 2);


	/*Quadratic Formula:
		(-b +- sqrt(discriminant)) / (2 * a)
	Where:
		discriminant = b^2 - 4ac
	*/
	float discrim = pow(b, 2) - 4 * a * c;

	if (discrim >= 0)
	{
		//2 solutions
		float distances[2] = {
			(-b + sqrt(discrim)) / (2 * a),
			(-b - sqrt(discrim)) / (2 * a),
		};

		for (short i = 0; i < 2; i++)
		{
			Vector3 hitPos = (Vector3){ 
				rayOrigin.x + rayDir.x * distances[i],
				rayOrigin.y + rayDir.y * distances[i],
				rayOrigin.z + rayDir.z * distances[i] 
			};
			Vector3 normal = (Vector3){
				hitPos.x - sphereOrigin.x,
				hitPos.y - sphereOrigin.y,
				hitPos.z - sphereOrigin.z
			};

			static float timer = 0;
			static int tDir = 1;
			timer += 0.000001f * 50 * tDir;
			if (timer > radius + 1)
				tDir = -1;
			else if (timer < -radius - 1)
				tDir = 1;
			//timer = WrapFloat(timer, -radius, radius);

			Vec3Normalize(&normal);
			Vector3 lightDir = { -1 , -1, timer };
			Vec3Normalize(&lightDir);

			float lightLevel = fmaxf(Vec3Dot(normal, Vec3MultiplyF(&lightDir, -1)), 0.0f);

			color = (Color32){
				//(normal.x * 0.5f + 0.5f) * 255,
				//WrapFloat(normal.x * 0.5f + 0.5f + timer, 0, 1) * 255,
				WrapFloat(lightLevel * 0.5f + 0.5f, 0, 1) * 255,
				WrapFloat(lightLevel * 0.5f + 0.5f, 0, 1) * 100,
				WrapFloat(lightLevel * 0.5f + 0.5f, 0, 1) * 10,
				255
			};
		}

		return color;
	}
	
	return (Color32){ 0 };
}


void RenderFrame(void)
{
	HDC deviceContext = GetDC(gMainWindow);
	SelectObject(deviceContext, (HFONT)GetStockObject(SYSTEM_FIXED_FONT));

	Transform* trans_ = (Transform*)GetComponent(DSID_Transform, &player);
	/*Pixel32 pixel = {0};
	pixel.blue = 175;
	pixel.green = 110;
	pixel.red = 100;
	pixel.alpha = 255;*/

	///*Pixel32*/__m128i quadPixel = {/**/ 175, 110, 100, 255,/**/ 175, 110, 100, 255,/**/ 175, 110, 100, 255,/**/ 175, 110, 100, 255};
	//__m128i quadPixel = {/**/ 56, 120, 72, 255,/**/ 56, 120, 72, 255,/**/ 56, 120, 72, 255,/**/ 56, 120, 72, 255};
	__m128i quadPixel = {/**/ 0x1e, 0x7f, 0x59, 255,/**/ 0x1e, 0x7f, 0x59, 255,/**/ 0x1e, 0x7f, 0x59, 255,/**/ 0x1e, 0x7f, 0x59, 255 };
	ClearScreen(quadPixel);

	DS_Draw();

#pragma region UNUSED Point-Slope Math
	/*Vector2 screenPos = {0};//Input.mouse.pixelPosition;
	//Vector2 screenPos = new_Vector2(trans_->positionX, trans_->positionY);//{ 0 };//Input.mouse.pixelPosition;

	bool undefinedSlope = false;
	float slope = 0;
	{
		float slpY = (screenPos.y + Input.mouse.pixelPosition.y);
		float slpX = (screenPos.x + Input.mouse.pixelPosition.x);

		if (slpY == 0)
			slope = 0;
		else if (slpX == 0)
			undefinedSlope = true;
		else
			slope = slpY / slpX;//1;
	}
	Vector2 screenScl = new_Vector2(RES_WIDTH, RES_HEIGHT);

	int startingPixel = ((RES_WIDTH * RES_HEIGHT) - RES_WIDTH) - \
		(RES_WIDTH * +screenPos.y) + screenPos.x;
	int pixIndex = 0;
	static double timer = 0;
	timer += gTime.deltaTime;

	Pixel32 pixel = { 255, 50, 50, 255 };
	for (int y = 0; y < screenScl.y; y++)
	{
		for (int x = 0; x < screenScl.x; x++)
		{
			if (screenPos.x + x < 0 || screenPos.x + x >= RES_WIDTH)
				continue;

			pixIndex = startingPixel + x - (RES_WIDTH * y);
			if (pixIndex < 0 || pixIndex >= RES_WIDTH * RES_HEIGHT)
				continue;

			float argX = (screenPos.x - x);// + Input.mouse.pixelPosition.x);
			float argY = (screenPos.y - y);// + Input.mouse.pixelPosition.y);

			if ((!undefinedSlope && slope == 0 && y == 0) || (undefinedSlope && x == 0) ||
				//(argX != 0 && abs(slope - (argY / argX)) != 0))
				(!undefinedSlope && (slope == argY / argX || (x == 0 && y == 0) )))
			{
				memcpy((Pixel32*)gBackBuffer.Memory + (uintptr_t)startingPixel + x - ((uintptr_t)RES_WIDTH * y), &pixel, sizeof(Pixel32));
				//memset((Pixel32*)gBackBuffer.Memory + (uintptr_t)startingPixel + x - ((uintptr_t)RES_WIDTH * y), /*Color* /0xFF, sizeof(Pixel32));
			}
		}
	}*/
#pragma endregion
	
#pragma region Cherno Raycasting
	//Pixel32 color = { 255, 50, 50, 255 };
	//int startingPixel = ((RES_WIDTH * RES_HEIGHT) - RES_WIDTH) - (RES_WIDTH * 0/*y*\/) + 0/*x*\/;
	//*
	int i = 0;
	int memOffset = 0;
	Color32 colors[4];

	while (false)//Disabled
	for (int y = 0; y < RES_HEIGHT; y++)
	{
		if (gTime.totalFramesRendered < 1)
			break;
		for (int x = 0; x < RES_WIDTH; x++)
		{
			Vector2 coord = { 
				x, RES_HEIGHT - y
				//(float)x / (RES_WIDTH + sizeof(Pixel32)),//For 3D
				//(float)y / (RES_HEIGHT + sizeof(Pixel32))
			};

			{
				//Vector2F mult = { coord.x * 2 - 1 , coord.y * 2 - 1 };//For 3D
				//coord = mult;//map <0,1> to <-1,1>
			}


			colors[i] = PerPixel(coord);
			//colors[i] = _PerPixel3D(coord);
			AlphaBlendColor32(&colors[i], (Color32*)gBackBuffer.Memory + x + (y * RES_WIDTH), AC_SrcOver);
			i++;

			if (i == 4)
			{
				_mm_store_si128((Color32*)gBackBuffer.Memory + memOffset, 
					(__m128i) {
						colors[0].blue, colors[0].green, colors[0].red, colors[0].alpha,
						colors[1].blue, colors[1].green, colors[1].red, colors[1].alpha,
						colors[2].blue, colors[2].green, colors[2].red, colors[2].alpha,
						colors[3].blue, colors[3].green, colors[3].red, colors[3].alpha
				});
				memOffset += i;
				i = 0;
			}

			//if (color.alpha == 0)
				//continue;

			//memcpy_s((Pixel32*)gBackBuffer.Memory + x + (y * RES_WIDTH), sizeof(Pixel32), &color, sizeof(Pixel32));
		}// *\/
	}// */
#pragma endregion


#ifdef OPENGL
	
	glViewport((gMonitorWidth / 2) - ((RES_WIDTH /** gViewData.curScaleFactor*/) / 2),
		(gMonitorHeight / 2) - ((RES_HEIGHT /** gViewData.curScaleFactor*/) / 2),
		RES_WIDTH /** gViewData.curScaleFactor*/, RES_HEIGHT /** gViewData.curScaleFactor*/);

	//Copy back buffer texture from memory
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, RES_WIDTH, RES_HEIGHT, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, gBackBuffer.Memory);
	//Set Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLES);

	//Bottom Tri
	//glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(-1, -1);//BL
	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(1, -1);//BR
	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(1, 1);//TR

	//Top Tri
	//glColor3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(-1, -1);//BL
	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(1, 1);//TR
	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(-1, 1);//TL

	glEnd();
	
	SwapBuffers(deviceContext);

#else

	StretchDIBits(deviceContext, 0, 0, gMonitorWidth, gMonitorHeight,
		0, 0, RES_WIDTH, RES_HEIGHT, gBackBuffer.Memory, &gBackBuffer.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

#endif

	//Show Debug Info
	if (gDebug.showDebugText == true)
	{
		const int ROW_HEIGHT = 20;

		char text[64] = { 0 };
		int r = 0;
		//SYSTEM
		snprintf(text, _countof(text), "Handles: %lu", gDebug.handleCount);
		TextOutA(deviceContext, 1920 - strlen(text) * 10, ROW_HEIGHT * r, text, (int)strlen(text));
		r++;

		snprintf(text, _countof(text), "Memory: %i KB", (int)(gDebug.memInfo.PrivateUsage / 1024));
		TextOutA(deviceContext, 1920 - strlen(text) * 10, ROW_HEIGHT * r, text, (int)strlen(text));
		r++;

		/*snprintf(text, _countof(text), "CPU: %.02f%%", gDebug.cpuPercent);//BROKEN
		TextOutA(deviceContext, 1920 - strlen(text) * 10, ROW_HEIGHT * r, text, (int)strlen(text));
		r++;*/
		//

		r = 0;
		snprintf(text, _countof(text), "FPS: %.01f (%.01f)", gTime.realFPSAverage, gTime.rawFPSAverage);
		TextOutA(deviceContext, 0, ROW_HEIGHT * r, text, (int)strlen(text));

		/*snprintf(text, _countof(text), "Timer: %.0f", timer);
		TextOutA(deviceContext, 0, ROW_HEIGHT * 16, text, (int)strlen(text));*/
		
		r++;
		snprintf(text, _countof(text), "Entity Count: %i/%i", gGameManager.numOfEntities, ENTITY_LIMIT);
		TextOutA(deviceContext, 0, ROW_HEIGHT * r, text, (int)strlen(text));

		r++;
		snprintf(text, _countof(text), "Tile Count: %i/%i", gGameManager.numOfTiles, TILE_LIMIT);
		TextOutA(deviceContext, 0, ROW_HEIGHT * r, text, (int)strlen(text));

		r++;
		r++;
		snprintf(text, _countof(text), "Player Pos: (%i, %i)", trans_->position.x, trans_->position.y);
		TextOutA(deviceContext, 0, ROW_HEIGHT * r, text, (int)strlen(text));

		r++;
		snprintf(text, _countof(text), "Mouse Pos: (%i, %i)", Input.mouse.pixelPosition.x, Input.mouse.pixelPosition.y);
		TextOutA(deviceContext, 0, ROW_HEIGHT * r, text, (int)strlen(text));

		/*snprintf(text, _countof(text), "Pixel: %i/%i", pixPos, RES_WIDTH * RES_HEIGHT);
		TextOutA(deviceContext, 0, ROW_HEIGHT * 2, text, (int)strlen(text));*/

		/*snprintf(text, _countof(text), "Timers: %.02f (Min: %.02f | Max: %.02f)",
			gTime.curTimerRes / 10000.0f,
			gTime.minTimerRes / 10000.0f,
			gTime.maxTimerRes / 10000.0f);
		TextOutA(deviceContext, 0, ROW_HEIGHT * 1, text, (int)strlen(text));*/
}

	ReleaseDC(gMainWindow, deviceContext);
}

//Bitmap Functions//

DWORD Load32BppBitmapFromFile(_Inout_ GAMEBITMAP* bitmap, _In_ char* filePath)
{
	DWORD error = ERROR_SUCCESS;
	HANDLE fileHandle = INVALID_HANDLE_VALUE;

	WORD bitmapHeader = 0;
	DWORD pixelDataOffset = 0;
	//DWORD numOfBytesToRead = 2;//the initial amount of bytes you want to read
	DWORD numOfBytesRead = 2;//the initial amount of bytes you want to read

	fileHandle = CreateFileA((LPCSTR)filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		error = GetLastError();
		goto Exit;
	}

	//if (ReadFileEx(fileHandle, &bitmapHeader, &numOfBytesToRead, NULL/*null == not async*/, NULL) == FALSE)
	if (ReadFile(fileHandle, &bitmapHeader, 2, &numOfBytesRead, NULL) == FALSE)
	{
		error = GetLastError();
		goto Exit;
	}

	//Is valid file
	if (bitmapHeader != 0x4d42)// "BM" backwards
	{
		error = ERROR_FILE_INVALID;
		goto Exit;
	}

	//Move read point to 10 (pixel data)
	if (SetFilePointer(fileHandle, 0xA, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		error = GetLastError();
		goto Exit;
	}

	//Pixel data reads the image from Left to Right, from Bottom to Top
	if (ReadFile(fileHandle, &pixelDataOffset, sizeof(DWORD), &numOfBytesRead, NULL) == FALSE)
	{
		error = GetLastError();
		goto Exit;
	}

	//Move read point to 14 (header data)
	if (SetFilePointer(fileHandle, 0xE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		error = GetLastError();
		goto Exit;
	}

	if (ReadFile(fileHandle, &bitmap->bitmapInfo.bmiHeader, sizeof(BITMAPINFOHEADER), &numOfBytesRead, NULL) == FALSE)
	{
		error = GetLastError();
		goto Exit;
	}

	//Allocate memory
	if ((bitmap->Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bitmap->bitmapInfo.bmiHeader.biSizeImage)) == NULL)
	{
		error = ERROR_NOT_ENOUGH_MEMORY;
		goto Exit;
	}

	//Move to pixel data
	if (SetFilePointer(fileHandle, pixelDataOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		error = GetLastError();
		goto Exit;
	}
	//Fill memory with pixel data
	if (ReadFile(fileHandle, bitmap->Memory, bitmap->bitmapInfo.bmiHeader.biSizeImage, &numOfBytesRead, NULL) == FALSE)
	{
		error = GetLastError();
		goto Exit;
	}
	//memcpy_s(bitmap->Memory, bitmap->bitmapInfo.bmiHeader.biSizeImage, )


Exit:
	if (fileHandle && fileHandle != INVALID_HANDLE_VALUE)
		CloseHandle(fileHandle);

	return error;
}

void LoadSpriteFromSheet(GAMEBITMAP* dest, GAMEBITMAP* sheet, int xOffset, int yOffset, int width, int height, int pivX, int pivY, bool flipped)
{
	if ((dest->Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Color32) * width * height)) == NULL)
	{
		exit(420);
	}

	yOffset = sheet->bitmapInfo.bmiHeader.biHeight - 1 - yOffset;
	int startPix = yOffset * sheet->bitmapInfo.bmiHeader.biWidth + xOffset;
	//int startPix = GetStartingPixel(sheet->bitmapInfo.bmiHeader.biWidth, sheet->bitmapInfo.bmiHeader.biHeight, xOffset, yOffset);
	/*for (size_t i = 0; i < width * height; i++)
	{
		memcpy_s((Pixel32*)dest->Memory + i, sizeof(Pixel32), (Pixel32*)sheet->Memory + startPix + i, sizeof(Pixel32));
	}*/

	int dir = -1;
	int startX = width;
	if (flipped)
	{
		dir = 1;
		startX = 0;
	}

	int i = 0;
	for (int yPix = 0; yPix < height + 1; yPix++)
	{
		if (yPix < height)
		{
			//for (int xPix = 0; xPix < width; xPix++)//
			for (int xPix = startX; xPix > 0 - (1 * ClampInt(dir, 0, 1)) && xPix < width - (1 * ClampInt(dir, -1, 0)); xPix += dir)//Loop forwards to flip sprite
			{
				Vector2 coord = {
					xPix , height - yPix
					//(float)x / (RES_WIDTH + sizeof(Pixel32)),//For 3D
					//(float)y / (RES_HEIGHT + sizeof(Pixel32))
				};

				int sheetOffset = startPix + (xPix - (sheet->bitmapInfo.bmiHeader.biWidth * yPix));
				int sliceOffset = width * height - i;//Start at the end and fill backwards
				memcpy_s((Color32*)((Color32*)dest->Memory + sliceOffset), sizeof(Color32), (Color32*)sheet->Memory + sheetOffset, sizeof(Color32));
				i++;
			}
		}
		else
			i += width;
		//This solution is hacky, but it works
		//Fixes a bug where the first pixel in each row is copied from the wrong spot
		//The "height + 1" and "i += width", fixes a bug where the bottom left pixel is skipped entirely
		if (gTime.totalFramesRendered > 0) //&& yPix * 12 < gTime.totalFramesRendered)//Show solution in real time
		{
			int sheetOffset = startPix + (0 - (sheet->bitmapInfo.bmiHeader.biWidth * yPix)) + sheet->bitmapInfo.bmiHeader.biWidth;
			int sliceOffset = width * height - i + width;
			memcpy_s((Color32*)((Color32*)dest->Memory + sliceOffset), sizeof(Color32), (Color32*)sheet->Memory + sheetOffset, sizeof(Color32));
		}
	}

	//Match the info
	memcpy_s(&dest->bitmapInfo, sizeof(BITMAPINFO), &sheet->bitmapInfo, sizeof(BITMAPINFO));
	//Change some info
	dest->bitmapInfo.bmiHeader.biWidth = width;
	dest->bitmapInfo.bmiHeader.biHeight = height;
	dest->bitmapInfo.bmiHeader.biSizeImage = sizeof(Color32) * width * height;

	dest->pivotX = pivX;
	dest->pivotY = pivY;

	return;
}

//Replaced by ECD System (ECS but I don't like that name)
/*void Blit32BppBitmapToBuffer(_Inout_ GAMEBITMAP* bitmap, _In_ unsigned int x, _In_ unsigned int y)
{

}*/

Vector2LONG GetResolution(GAMEBITMAP* bitmap)
{
	Vector2LONG result = { 0 };
	result.x = bitmap->bitmapInfo.bmiHeader.biWidth;
	result.y = bitmap->bitmapInfo.bmiHeader.biHeight;

	return result;
}
void SetResolution(GAMEBITMAP* bitmap, long x, long y)
{
	bitmap->bitmapInfo.bmiHeader.biWidth = x;
	bitmap->bitmapInfo.bmiHeader.biHeight = y;
}

ColorF Color32ToColorF(Color32* color)
{
	ColorF c = { 0 };
	c.BGRA = (Vector4) { color->blue / 255.0f, color->green / 255.0f, color->red / 255.0f, color->alpha / 255.0f };

	return c;
}

void AlphaBlendColor32(_Inout_ Color32* over_, _In_ const Color32* og_, _In_ const AC_BlendModes MODE)
{
	//Check if you need to blend
	if (over_->alpha >= 255)//overlay is fully opaque
		return;
	else if (over_->alpha <= 0)//overlay is fully transparent
	{
		memcpy_s(over_, sizeof(Color32), og_, sizeof(Color32));
		return;
	}

	if (MODE == AC_SrcOver)
	{
		ColorF backColor = Color32ToColorF(og_);
		ColorF frontColor = Color32ToColorF(over_);
		ColorF finalColor = { 0 };

		Vector3 v3 = Vec3MultiplyF(&backColor.BGRA, backColor.alpha);
		backColor.BGRA = Vec4FromVec3(&v3, backColor.alpha);

		v3 = Vec3MultiplyF(&frontColor.BGRA, frontColor.alpha);
		frontColor.BGRA = Vec4FromVec3(&v3, frontColor.alpha);

		finalColor.BGRA = Vec4Add(&frontColor, Vec4MultiplyF(&backColor.BGRA, (1 - frontColor.alpha)));
		v3 = Vec3MultiplyF(&finalColor.BGRA, finalColor.alpha);
		finalColor.BGRA = Vec4FromVec3(&v3, finalColor.alpha);

		memcpy_s(over_, sizeof(Color32),
			&(Color32) {
				finalColor.blue * 255,
				finalColor.green * 255,
				finalColor.red * 255,
				finalColor.alpha * 255,
			}, sizeof(Color32));
		/*return (Color32) {
			finalColor.blue * 255,
			finalColor.green * 255,
			finalColor.red * 255,
			finalColor.alpha * 255,
		};*/
	}
}

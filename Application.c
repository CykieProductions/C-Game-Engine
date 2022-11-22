#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <emmintrin.h>
//My Stuff
#include "Application.h"
#include "Renderer.h"
//#include "Cytools.h"
#include "DriverSystem.h"
//#include "Components.h"

//globals

bool gIsPlaying = true;
HWND gMainWindow = { 0 };
char GAME_NAME[64] = "DEFAULT_NAME";
wchar_t WGAME_NAME[sizeof(GAME_NAME)];

MONITORINFO gMonitorInfo = { sizeof(MONITORINFO) };
int gMonitorWidth = 0;
int gMonitorHeight = 0;
APP_INPUT Input = { 0 };

Entity player = { 0 };
//

int WinMain(HINSTANCE hInstance, HINSTANCE dep_hPrevInst, PSTR lpCmdLine, int nCmdShow)
{
#pragma region Game Setup
    {
        char newName[] = "Test Game";
        SetString(GAME_NAME, newName, sizeof(GAME_NAME), sizeof(newName));

        char path[64] = "Assets/valid.txt";
        char backStr[] = "../";
        int r = 0;
        while (true)
        {
            if (r >= 5 || IsFileValid(path) == true)
                break;
            char tmp[64];
            SetString(tmp, path, sizeof(tmp), sizeof(path));
            ConcatStrings(path, sizeof(path), backStr, tmp);//Keep going up a folder until you find the Assets folder
            r++;
        }
        for (size_t i = strlen(path); i > 0; i--)//Remove "valid.txt" from the string
        {
            if (path[i] == '/' || path[i] == '\\')
                break;
            path[i] = '\0';
        }

        //char path[] = "../../Assets/";
        //char path[] = "C:/Users/crayb/OneDrive/Documents/Cloud Programming/C/C-Game-Engine/Assets/";
        SetString(gDebug.defaultDataPath, path, sizeof(gDebug.defaultDataPath), sizeof(path));
    }

    //Insta-Crash Conditions
    if (IsGameAlreadyRunning() == TRUE)
    {
        MessageBox(NULL, L"An instance of this program is already running!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }
    else if (InitBackBuffer() == FALSE)
    {
        MessageBox(NULL, L"Failed to allocate memory for drawing area!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }
    else if (CreateMainWindow(hInstance) != ERROR_SUCCESS)
    {
        goto Exit;
    }
    Input.mouse.cursorInfo.hCursor = GetCursor();

    //Timer Resolution Setup
    HMODULE NtDllModHandle;
    if ((NtDllModHandle = GetModuleHandleA("ntdll.dll")) == NULL)
    {
        MessageBox(NULL, L"Failed to load ntdll.dll!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
}

    if ((NtQueryTimerResolution = (_NtQueryTimerResolution)GetProcAddress(NtDllModHandle, "NtQueryTimerResolution")) == NULL)
    {
        MessageBox(NULL, L"Couldn't find the \"NtQueryTimerResolution\" function in ntdll.dll!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    NtQueryTimerResolution(&gTime.minTimerRes, &gTime.maxTimerRes, &gTime.curTimerRes);
    GetSystemInfo(&gDebug.systemInfo);
    //

#ifdef OPENGL
    if (InitOpenGL() != ERROR_SUCCESS)
    {
        MessageBox(NULL, L"Failed to Initialize OpenGL!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
    }
#endif // OPENGL

    QueryPerformanceFrequency((LARGE_INTEGER*)&gTime.perfFrequency);//Only once

    InitTimeData();
    GetSystemTimeAsFileTime((FILETIME*)&gDebug.prevSystemTime);

    char filePath[128];
    ConcatStrings(filePath, sizeof(filePath), gDebug.defaultDataPath, "char_spritesheet.bmp");

    if (Load32BppBitmapFromFile(&gGameManager.spriteCache[SCE_PlayerSS]/*&gGameManager.playerSpriteSheet*/,
        filePath) != ERROR_SUCCESS)
    {
        MessageBox(NULL, L"Failed to load bitmap!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        gIsPlaying = FALSE;
    }
    
    ConcatStrings(filePath, sizeof(filePath), gDebug.defaultDataPath, "Overworld_Tileset.bmp");
    if (Load32BppBitmapFromFile(&gGameManager.spriteCache[SCE_OverworldTileset1],
        filePath) != ERROR_SUCCESS)
    {
        MessageBox(NULL, L"Failed to load bitmap!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        gIsPlaying = FALSE;
    }
#pragma endregion

#pragma region Game Logic
    MSG message = { 0 };
    message.hwnd = gMainWindow;
    uint64_t frameStart = { 0 };
    uint64_t frameEnd = { 0 };
    uint64_t rawElapseMicrosecondsAccumulator = { 0 };
    uint64_t realElapseMicrosecondsAccumulator = { 0 };

    HANDLE curProcess = GetCurrentProcess();
    FILETIME processCreationTime = { 0 };
    FILETIME processExitTime = { 0 };

    gIsPlaying = TRUE;

    //START////////////////////////////////////////////////////////////////
    //InitPlayer(25, 25, 1, 1);

    //Scene loading test
    FILE* file_;// = malloc(sizeof(void*));
    char path[128] = { 0 };
    ConcatStrings(path, sizeof(path), gDebug.defaultDataPath, "Scenes/Testing.scn");
    fopen_s(&file_, path, "r");
    

    if (file_ == NULL)
    {
        InitPlayer(25, 25, 1, 1);

        //Save Scene
        fopen_s(&file_, path, "a+");
        if (file_ != NULL)
        {
            fwrite(&player, sizeof(player), 1, file_);
        }

        if (file_ != NULL)
        {
            for (size_t i = 0; i < sizeof(player.components) / sizeof(ComponentInfo); i++)
            {
                fwrite(player.components[i].memory, player.components[i].size, 1, file_);
                //DS_CopyComponent(player.components[i].memory, &player, &player.components[i])//Filter yourself
            }
        }

        /*for (size_t i = 0; i < gGameManager.allTiles; i++)
         {

         }*/
    }
    else
    {
        fread(&player, sizeof(player), 1, file_);

        //Found a way to avoid just copying the old memory locations
        //Each component needed to be saved and loaded
        //And then they're re-registered like in CloneEntity()

        /**/ComponentInfo tempComps[sizeof(player.components) / sizeof(ComponentInfo)];
        memcpy_s(&tempComps, sizeof(tempComps), &player.components, sizeof(player.components));

        for (size_t i = 0; i < sizeof(player.components) / sizeof(ComponentInfo); i++)
        {
            if (tempComps[i].memory == NULL)
                continue;
            tempComps[i].memory = malloc(tempComps[i].size);//New memory
            fread(tempComps[i].memory, tempComps[i].size, 1, file_);//fill new memory with saved data
            //Debug
            //long cursorPos = ftell(file_);
            //int tmp = 0;
        }

        DS_RegisterEntity(&player);
        FillArrayWithValue(&player.components, sizeof(player.components), NULL, sizeof(NULL));
        for (size_t i = 0; i < sizeof(player.components) / sizeof(ComponentInfo); i++)
        {
            if (tempComps[i].memory != NULL && tempComps[i].size != 0)
            {
                AddComponent(tempComps[i].dsID, &player);
                DS_CopyComponent(player.components[i].memory, &player, &tempComps[i]);
            }
        }
        //*/
    }
    if (file_ != NULL)
        fclose(file_);//(&outfile);

    
    Entity* entity_ = InitEntity(OT_Entity);
    Transform* entTrans_ = AddComponent(DSID_Transform, entity_);
    AddComponent(DSID_Collider, entity_);
    entTrans_->position.x = 100;
    entTrans_->position.y = 10;

    SpriteRenderer* entRend_ = AddComponent(DSID_SpriteRenderer, entity_);
    LoadSpriteFromSheet(&entRend_->sprite, &gGameManager.spriteCache[0], 128 + 16, 16, 16, 16, 0, 0, false);
    {
        Animator* anim_ = AddComponent(DSID_Animator, entity_);
        FillArrayWithValue(&anim_->clips, sizeof(anim_->clips), NULL, sizeof(AnimationClip));
        int cIndex = 0;

        AnimationClip* clip_ = &anim_->clips[cIndex];
        SetString(clip_->name, "Idle", sizeof(clip_->name), sizeof("Idle"));
        clip_->variant = ANIM_DOWN;
        clip_->frameCount = 5;
        clip_->defaultDelay = 0.1f;
        for (size_t i = 0; i < clip_->frameCount; i++)
        {
            SpriteSliceData* frameData = &clip_->frames[i];
            frameData->sourceIndex = SCE_PlayerSS;//gGameManager.playerSpriteSheet;
            frameData->slicePosX = 128 + 64 + (16 * i);
            frameData->slicePosY = 96;
            frameData->sliceWidth = 16;
            frameData->sliceHeight = 16;
            frameData->pivotX = 0;
            frameData->pivotY = 0;
        }

        Animator_Play(anim_, "Idle", 0, true, 0, false);
    }

        Entity* tilesetEnt_ = InitEntity(OT_Gizmo);
        Transform* tilesetTrans_ = AddComponent(DSID_Transform, tilesetEnt_);
    {
        tilesetTrans_->position.x = -1;
        tilesetTrans_->position.y = -1;

        SpriteRenderer* tilesetRend_ = AddComponent(DSID_SpriteRenderer, tilesetEnt_);
        tilesetRend_->sortingLayer = SortLayer_UI;
        tilesetRend_->orderInLayer = -55;
        tilesetRend_->sprite = gGameManager.spriteCache[SCE_OverworldTileset1];
        //LoadSpriteFromSheet(&tilesetRend_->sprite, &gGameManager.spriteCache[SCE_OverworldTileset1], 0, 16, 16, 16, 0, 0, false);
    }

    SpriteSliceData ssd = new_SpriteSliceData(SCE_OverworldTileset1, false, false,
        0, 0, 16, 16, 0, 0);
    Transform* brushTrans_ = new_TileBrush(&gGameManager.spriteCache[SCE_OverworldTileset1], ssd);
    SpriteRenderer* brushRend_ = GetComponent(DSID_SpriteRenderer, brushTrans_->entity);
    brushRend_->transparencyPercent = 0.5f;
    brushRend_->sortingLayer = SortLayer_UI;
    brushRend_->orderInLayer = -50;
    
    POINT lockedMousePoint = { 0 };

    DS_Update(UPDATE_MODE_START);

    //!/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //!GAME LOOP////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while (gIsPlaying == TRUE)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&frameStart);

        Input.mouse.deltaScroll = 0;//Must reset at the start of game loop
        while (PeekMessageA(&message, gMainWindow, 0, 0, PM_REMOVE))
        {
            //TranslateMessage(&message);
            DispatchMessage(&message);
        }
        if (gDebug.isInEditMode)
        {

        }
        else
        {
            ProcessPlayerInput();

            static SortingLayer activeSortingLayer = { 0 };
            if (Input.keyboard.number0.pressed)
                activeSortingLayer = SortLayer_YZ;
            else if (Input.keyboard.number1.pressed)
                activeSortingLayer = SortLayer_Back;
            else if (Input.keyboard.number2.pressed)
                activeSortingLayer = SortLayer_Front;

            //tileRend_->defaultSprite.flipSprite = true;
            //tileRend_->defaultSprite.pivotX = -1;

            if (!Input.keyboard.qKey.held)
            {
                tilesetEnt_->disabled = true;
                if (Input.keyboard.qKey.released)
                {
                    SetCursorPos(lockedMousePoint.x, lockedMousePoint.y);
                }
                else
                {
                    brushTrans_->position.x = Input.mouse.pixelPosition.x - (Input.mouse.pixelPosition.x % 16) - 1;
                    brushTrans_->position.y = Input.mouse.pixelPosition.y - (Input.mouse.pixelPosition.y % 16) - 1;
                }
            }
            else
            {
                tilesetEnt_->disabled = false;
                if (Input.keyboard.qKey.pressed)
                {
                    lockedMousePoint = Input.mouse.cursorInfo.ptScreenPos;

                    //tilesetTrans_->positionX = tileTrans_->positionX - tileRend_->defaultSprite.slicePosX;
                    //tilesetTrans_->positionY = tileTrans_->positionY - tileRend_->defaultSprite.slicePosY;
                    //SetCursorPos(0, 0);
                }

                if (fabsf(Input.mouse.deltaMove.x) > 0)// && !Input.keyboard.shiftKey.held)
                    brushRend_->defaultSprite.slicePosX = Input.mouse.pixelPosition.x - \
                        (Input.mouse.pixelPosition.x % 16);//16 * SnapFloatTo(Input.mouse.moveDir.x - .5f, -1, 1);//-Input.mouse.scrollDir;

                if (fabsf(Input.mouse.deltaMove.y) > 0)// && Input.keyboard.shiftKey.held)
                    brushRend_->defaultSprite.slicePosY = Input.mouse.pixelPosition.y - \
                        (Input.mouse.pixelPosition.y % 16);//16 * (SnapFloatTo(Input.mouse.moveDir.y - .5f, 0, 1) * 2 - 1);//-Input.mouse.scrollDir;
            }

            if (fabsf(Input.mouse.deltaScroll) > 0 && !Input.keyboard.shiftKey.held)
                brushRend_->defaultSprite.slicePosX += 16 * -Input.mouse.scrollDir;
            else if (fabsf(Input.mouse.deltaScroll) > 0)// && Input.keyboard.shiftKey.held)
                brushRend_->defaultSprite.slicePosY += 16 * -Input.mouse.scrollDir;
            

            //! X Scrolling Limits
            if (brushRend_->defaultSprite.slicePosX >=
                gGameManager.spriteCache[brushRend_->defaultSprite.sourceIndex].bitmapInfo.bmiHeader.biWidth)
            {
                if (brushRend_->defaultSprite.slicePosY + 16 >=
                    gGameManager.spriteCache[brushRend_->defaultSprite.sourceIndex].bitmapInfo.bmiHeader.biHeight)
                {
                    brushRend_->defaultSprite.slicePosX =
                        gGameManager.spriteCache[brushRend_->defaultSprite.sourceIndex].bitmapInfo.bmiHeader.biWidth - 16;
                    //tileRend_->defaultSprite.slicePosY =
                        //gGameManager.spriteCache[tileRend_->defaultSprite.sourceIndex].bitmapInfo.bmiHeader.biHeight - 16;
                }
                else
                {
                    brushRend_->defaultSprite.slicePosX =
                        gGameManager.spriteCache[brushRend_->defaultSprite.sourceIndex].bitmapInfo.bmiHeader.biWidth - 16;
                    //tileRend_->defaultSprite.slicePosX = 0;
                    //tileRend_->defaultSprite.slicePosY += 16;
                }
            }
            else if (brushRend_->defaultSprite.slicePosX < 0)
            {
                if (brushRend_->defaultSprite.slicePosY <= 0)
                {
                    brushRend_->defaultSprite.slicePosX = 0;
                    //tileRend_->defaultSprite.slicePosY = 0;
                }
                else
                {
                    brushRend_->defaultSprite.slicePosX = 0;
                    //Wrapping
                    //tileRend_->defaultSprite.slicePosX =
                        //gGameManager.spriteCache[tileRend_->defaultSprite.sourceIndex].bitmapInfo.bmiHeader.biWidth - 16;
                    //tileRend_->defaultSprite.slicePosY -= 16;
                }
            }
            //! Y Scrolling Limits
            if (brushRend_->defaultSprite.slicePosY >=
                gGameManager.spriteCache[brushRend_->defaultSprite.sourceIndex].bitmapInfo.bmiHeader.biHeight)
            {
                brushRend_->defaultSprite.slicePosY =
                    gGameManager.spriteCache[brushRend_->defaultSprite.sourceIndex].bitmapInfo.bmiHeader.biHeight - 16;
            }
            else if (brushRend_->defaultSprite.slicePosY < 0)
            {
                brushRend_->defaultSprite.slicePosY = 0;
            }


            LoadSpriteFromSheet(&brushRend_->sprite, &gGameManager.spriteCache[brushRend_->defaultSprite.sourceIndex], 
                brushRend_->defaultSprite.slicePosX, 
                brushRend_->defaultSprite.slicePosY,
                brushRend_->defaultSprite.sliceWidth, brushRend_->defaultSprite.sliceHeight,
                brushRend_->defaultSprite.pivotX, brushRend_->defaultSprite.pivotY, brushRend_->defaultSprite.flipSprite);

            if (Input.mouse.leftButtonPressed)
            {
                Transform* tileTrans_ = PaintTile(brushTrans_->entity);
                SpriteRenderer* tile_ = tileTrans_->entity->components[0].memory;

                tile_->sortingLayer = activeSortingLayer;
            }
            {
                /*HDC deviceContext = GetDC(gMainWindow);
                  SelectObject(deviceContext, (HFONT)GetStockObject(SYSTEM_FIXED_FONT));
                  const int ROW_HEIGHT = 20;

                  char text[64] = { 0 };
                  int r = 16;

                  snprintf(text, _countof(text), "Active Layer: %i", activeSortingLayer);
                  TextOutA(deviceContext, 0, ROW_HEIGHT* r, text, (int)strlen(text));*/
            }

            /*if (Input.mouse.middleButtonReleased)
             {
                for (size_t i = 0; i < ENTITY_LIMIT; i++)
                {
                    if (gGameManager.allEntities[i] == NULL || gGameManager.allEntities[i] == &player || gGameManager.allEntities[i] == entity_)
                        continue;
                    if (DestroyEntity(gGameManager.allEntities[i]))
                        break;
                }
             }*/
            //if (gTime.totalFramesRendered % 2 == 0)
            //if (gTime.totalFramesRendered < 999)
            //if (Input.mouse.middleButtonPressed)
            if (false)
            {
                bool loop = true;
                //loop = rand() % 101 + 1 > 50;

                Entity* clone_ = CloneEntity(entity_);

                if (clone_ != NULL)
                {
                    srand((int)(frameStart % (gTime.totalFramesRendered + 1)));
                    ((Transform*)clone_->components[0].memory)->position.x = Input.mouse.pixelPosition.x; //+= 16 + (rand() % 180 + 1);
                    ((Transform*)clone_->components[0].memory)->position.y = Input.mouse.pixelPosition.y; //+= 2 + (rand() % 180 + 1);
                    Animator_Play(((Animator*)clone_->components[2].memory), "Idle", 0, loop, 0, false);
                }
            }

            

            DS_Update(UPDATE_MODE_NORMAL);
        }

        RenderFrame();

        DS_Update(UPDATE_MODE_LATE);

        #pragma region Frame Calculations
        QueryPerformanceCounter((LARGE_INTEGER*)&frameEnd);

        gTime.elapsedMicroSeconds = frameEnd - frameStart;//Elapsed ticks
        gTime.elapsedMicroSeconds *= SEC_TO_MIC;
        gTime.elapsedMicroSeconds /= gTime.perfFrequency;//Elapsed microseconds

        gTime.totalFramesRendered++;
        rawElapseMicrosecondsAccumulator += gTime.elapsedMicroSeconds;

        //Correct to target frame rate
        while (gTime.elapsedMicroSeconds < (1 / gTime.targetFPS) * SEC_TO_MIC)
        {
            gTime.elapsedMicroSeconds = frameEnd - frameStart;//Elapsed ticks
            gTime.elapsedMicroSeconds *= SEC_TO_MIC;
            gTime.elapsedMicroSeconds /= gTime.perfFrequency;//Elapsed microseconds

            QueryPerformanceCounter((LARGE_INTEGER*)&frameEnd);

            //Check if too much time would pass before this thread gets woke up again
            if (gTime.elapsedMicroSeconds < (gTime.targetMicroseconds - (gTime.curTimerRes * 0.1)) * 0.0058f)
                Sleep(0);
        }

        //Calculate deltaTime
        /*uint64_t newframeTime = {0};
        QueryPerformanceCounter((LARGE_INTEGER*)&newframeTime);*/
        gTime.deltaTime = (double)((frameEnd - frameStart) * MIC_TO_SEC * 0.1);


        realElapseMicrosecondsAccumulator += gTime.elapsedMicroSeconds;

        if (gTime.totalFramesRendered % gTime.framesBeforeAvgCalc == 0)//Every "100(?)" frames
        {
            //Debug Info Setup
            GetSystemTimeAsFileTime((FILETIME*)&gDebug.curSystemTime);
            GetProcessHandleCount(curProcess, &gDebug.handleCount);
            GetProcessMemoryInfo(curProcess, (PPROCESS_MEMORY_COUNTERS*)& gDebug.memInfo,
                sizeof(gDebug.memInfo));
            GetProcessTimes(curProcess, &processCreationTime, &processExitTime,
                (FILETIME*)&gDebug.curKernelCPUTime, (FILETIME*)&gDebug.curUserCPUTime);

            gDebug.cpuPercent = (float)(gDebug.curKernelCPUTime - gDebug.prevKernelCPUTime) + \
                (gDebug.curUserCPUTime - gDebug.prevUserCPUTime);

            gDebug.cpuPercent /= (gDebug.curSystemTime - gDebug.prevSystemTime);
            gDebug.cpuPercent /= gDebug.systemInfo.dwNumberOfProcessors;
            gDebug.cpuPercent *= 100;
            //

            int64_t rawAvgMicrosecondsPF = rawElapseMicrosecondsAccumulator / gTime.framesBeforeAvgCalc;
            int64_t realAvgMicrosecondsPF = realElapseMicrosecondsAccumulator / gTime.framesBeforeAvgCalc;

            gTime.rawFPSAverage = 1.0 / ((rawElapseMicrosecondsAccumulator / gTime.framesBeforeAvgCalc) * MIC_TO_SEC);
            gTime.realFPSAverage = 1.0 / ((realElapseMicrosecondsAccumulator / gTime.framesBeforeAvgCalc) * MIC_TO_SEC);

            /*OLD DEBUG*/ {
                char frameStats[128] = { 0 };
                //_snprintf_s(str, _countof(str), _TRUNCATE, "Elapsed microseconds: %lli\n", gTime.elapsedMicroSeconds);
                _snprintf_s(frameStats, _countof(frameStats), _TRUNCATE,
                    "Mil/Frame: %.01f\t\tFPS: %.01f\t\tRaw FPS: %.01f\t\tDelta: %.03f\n", rawAvgMicrosecondsPF * 0.001f,
                    gTime.realFPSAverage,
                    gTime.rawFPSAverage,
                    gTime.deltaTime);
                OutputDebugStringA(frameStats);
            }//

            realElapseMicrosecondsAccumulator = 0;
            rawElapseMicrosecondsAccumulator = 0;

            //More Debug Info Setup//
            gDebug.prevKernelCPUTime = gDebug.curKernelCPUTime;
            gDebug.prevUserCPUTime = gDebug.curUserCPUTime;
            gDebug.prevSystemTime = gDebug.curSystemTime;
            //
        }
        #pragma endregion
    }
    ///////////////////////////////////////////////////////////////////////
#pragma endregion

Exit:
    return 0;
}

#ifdef OPENGL
unsigned long InitOpenGL(void)
{
    unsigned long result = ERROR_SUCCESS;

#ifdef _WIN32
    HDC winDC = GetDC(gMainWindow);
    HGLRC openGLRenderContext = NULL;
    int pixelFormatIndex = 0;

    PIXELFORMATDESCRIPTOR pixelFormat = { sizeof(PIXELFORMATDESCRIPTOR) };

    pixelFormat.nVersion = 1;
    pixelFormat.iPixelType = PFD_TYPE_RGBA;
    pixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pixelFormat.cColorBits = 32;
    pixelFormat.cAlphaBits = 8;
    pixelFormat.iLayerType = PFD_MAIN_PLANE;

    pixelFormatIndex = ChoosePixelFormat(winDC, &pixelFormat);
    if (pixelFormatIndex == 0)
    {
        result = GetLastError();
        goto Exit;
    }

    DescribePixelFormat(winDC, pixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormat);
    SetPixelFormat(winDC, pixelFormatIndex, &pixelFormat);
    openGLRenderContext = wglCreateContext(winDC);
    if (openGLRenderContext == NULL)
    {
        result = GetLastError();
        goto Exit;
    }

    if (wglMakeCurrent(winDC, openGLRenderContext) == FALSE)
    {
        result = GetLastError();
        goto Exit;
    }

    ((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(1);//Enable Vsync if available

Exit:
    if (winDC)
        ReleaseDC(gMainWindow, winDC);

    return(result);

#else//Linux

#endif
}
#endif

DWORD CreateMainWindow(_In_ HINSTANCE hInstance)
{
    DWORD result = ERROR_SUCCESS;

#pragma region WindowClass Setup & Registration
    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);//Set in the Manifest Tools instead
    WNDCLASSEX windowClass = { 0 };
    //HWND windowHandle = 0;//replaced with global: gMainWindow
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = 0;
    windowClass.lpfnWndProc = MainWndProc;//Ties the dispatch to this window
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;//GetModuleHandle(NULL);//same
    windowClass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    windowClass.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);//small icon
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));//(HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = GameNameToWide();

    if (RegisterClassEx(&windowClass) == 0)
    {
        result = GetLastError();
        MessageBox(NULL, L"Failed to Register Window: " + result, L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }
#pragma endregion

    //Actual windows creation
    gMainWindow = CreateWindowEx(0, windowClass.lpszClassName, L"Win Title"
        ,WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

#pragma region Setup & Error Checking
    if (gMainWindow == NULL)
    {
        result = GetLastError();
        MessageBox(NULL, L"Failed to Register Window: " + result, L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (GetMonitorInfo(MonitorFromWindow(gMainWindow, MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == 0)
    {
        result = ERROR_MONITOR_NO_DESCRIPTOR;
        MessageBox(NULL, L"Failed to get monitor: " + result, L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    //This placement is important
    gMonitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;
    gMonitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;

    if (SetWindowLongPtr(gMainWindow, GWL_STYLE, WS_VISIBLE) == 0)
    {
        result = GetLastError();
        MessageBox(NULL, L"Failed to Set Window: " + result, L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }
    if (SetWindowPos(gMainWindow, HWND_TOP, gMonitorInfo.rcMonitor.left, gMonitorInfo.rcMonitor.top, gMonitorWidth, gMonitorHeight, SWP_NOOWNERZORDER/*?*/ | SWP_FRAMECHANGED) == 0)
    {
        result = GetLastError();
        MessageBox(NULL, L"Failed to Position Window: " + result, L"Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }
#pragma endregion

Exit:
    return result;
}

BOOL IsGameAlreadyRunning(void)
{
    HANDLE mutex = NULL;
    char mutexName[128] = { 0 };
    ConcatStrings(mutexName, sizeof(mutexName), GAME_NAME, "_GameMutex");

    mutex = CreateMutex(NULL, FALSE, (LPCWSTR)mutexName);

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

LPCWSTR GameNameToWide(void)
{
    //https://www.tutorialspoint.com/c_standard_library/c_function_mbstowcs.htm
    mbstowcs_s(NULL, WGAME_NAME, strlen(GAME_NAME) + 1, GAME_NAME, strlen(GAME_NAME));
    LPCWSTR result = WGAME_NAME;
    return result;
}

LRESULT CALLBACK MainWndProc(/*handle to window*/HWND hwnd, /*message identifier*/UINT uMsg, /*first message parameter*/WPARAM wParam, /*second message parameter*/LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg)
    {
    case WM_CREATE:
        // Initialize the window. 
        break;

    /*case WM_MOUSEMOVE: {
        //MOUSEINPUT mi = { 0 };
        PCURSORINFO ci_ = { 0 };
        HCURSOR mouse = GetCursor();
        GetCursorInfo(ci_);
        
        //mouse_event(mi.dwFlags, mi.dx, mi.dy, mi.mouseData, mi.dwExtraInfo);
        break;}*/

    case WM_ACTIVATE:
        if (wParam == 0)//Lost Focus
        {
            gDebug.windowHasFocus = FALSE;
        }
        else
        {
            gDebug.windowHasFocus = TRUE;
            //ShowCursor(FALSE);
        }
        break;

    case WM_MOUSEWHEEL: {
        Input.mouse.deltaScroll = 0;
        Input.mouse.deltaScroll = GET_WHEEL_DELTA_WPARAM(wParam);

        break;}

    /*case WM_PAINT: {//Keep the variables in scope (NOT OPTIONAL)
        // Paint the window's client area. 
        PAINTSTRUCT paint;
        HDC hdc = BeginPaint(hwnd, &paint);

        // All painting occurs here, between BeginPaint and EndPaint.

        //FillRect(hdc, &paint.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &paint);
        break;
    }*/
    case WM_SIZE:
        // Set the size and position of the window. 
        break;

    case WM_DESTROY:
        // Clean up window-specific data objects. 
        gIsPlaying = FALSE;
        PostQuitMessage(0);
        break;

        // 
        // Process other messages. 
        // 
    case WM_CLOSE:
        gIsPlaying = FALSE;
        PostQuitMessage(0);
        break;

    default:
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return result;
}

void App_SetButtonManually(bool* pressed, bool* released, bool* held, int vk)
{
    *pressed = !*held && GetAsyncKeyState(vk);
    *released = *held && !GetAsyncKeyState(vk);
    *held = GetAsyncKeyState(vk);
}
void App_SetButton(ButtonInput* button, int vk)
{
    button->pressed = !button->held && GetAsyncKeyState(vk);
    button->released = button->held && !GetAsyncKeyState(vk);
    button->held = GetAsyncKeyState(vk);
}
void App_SetDualButton(ButtonInput* target, ButtonInput* b1, ButtonInput* b2)
{
    target->pressed = b1->pressed || b2->pressed;
    target->released = b1->released || b2->released;
    target->held = b1->held || b2->held;
}

void ProcessPlayerInput(void)
{
    if (!gDebug.windowHasFocus)
        return;

#pragma region Mouse Input
    Input.mouse.deltaMove = (Vector2){ 0, 0 };
    Vector2INT prevMousePos = Input.mouse.pixelPosition;//(Vector2INT){ Input.mouse.cursorInfo.ptScreenPos.x, Input.mouse.cursorInfo.ptScreenPos.y };

    GetCursorPos(&Input.mouse.cursorInfo.ptScreenPos);
    Input.mouse.pixelPosition = (Vector2INT){
        Input.mouse.cursorInfo.ptScreenPos.x / (gMonitorWidth * 0.0026f),
        Input.mouse.cursorInfo.ptScreenPos.y / (gMonitorHeight * 0.004155f)
    };

    Input.mouse.deltaMove = Vec2Add((Vector2)
    {
        Input.mouse.pixelPosition.x,
        Input.mouse.pixelPosition.y,
    }, (Vector2)
    {
        -prevMousePos.x, -prevMousePos.y
    });

    Input.mouse.moveDir = Input.mouse.deltaMove;
    Vec2Normalize(&Input.mouse.moveDir);

    //Delta Scroll set in the WinProc function
    if (Input.mouse.deltaScroll > 0)
        Input.mouse.scrollDir = 1;
    else if (Input.mouse.deltaScroll < 0)
        Input.mouse.scrollDir = -1;
    else
        Input.mouse.scrollDir = 0;


    //if button pressed, but "ButtonHeld" isn't active yet -    TAPPED
    Input.mouse.leftButtonPressed = !Input.mouse.leftButtonHeld && GetAsyncKeyState(VK_LBUTTON);
    //if button not pressed, but "ButtonHeld" is still active - RELEASED
    Input.mouse.leftButtonReleased = Input.mouse.leftButtonHeld && !GetAsyncKeyState(VK_LBUTTON);
    //if button is pressed - HELD
    Input.mouse.leftButtonHeld = GetAsyncKeyState(VK_LBUTTON);

    Input.mouse.middleButtonPressed = !Input.mouse.middleButtonHeld && GetAsyncKeyState(VK_MBUTTON);//TAPPED
    Input.mouse.middleButtonReleased = Input.mouse.middleButtonHeld && !GetAsyncKeyState(VK_MBUTTON);//RELEASED
    Input.mouse.middleButtonHeld = GetAsyncKeyState(VK_MBUTTON);//HELD

    Input.mouse.rightButtonPressed = !Input.mouse.rightButtonHeld && GetAsyncKeyState(VK_RBUTTON);//TAPPED
    Input.mouse.rightButtonReleased = Input.mouse.rightButtonHeld && !GetAsyncKeyState(VK_RBUTTON);//RELEASED
    Input.mouse.rightButtonHeld = GetAsyncKeyState(VK_RBUTTON);//HELD
#pragma endregion

#pragma region Letter Keys

    App_SetButtonManually(
        &Input.keyboard.upKey.pressed, 
        &Input.keyboard.upKey.released, 
        &Input.keyboard.upKey.held, 
        VK_UP);
    App_SetButtonManually(
        &Input.keyboard.downKey.pressed, 
        &Input.keyboard.downKey.released, 
        &Input.keyboard.downKey.held,
        VK_DOWN);
    App_SetButtonManually(
        &Input.keyboard.leftKey.pressed, 
        &Input.keyboard.leftKey.released, 
        &Input.keyboard.leftKey.held,
        VK_LEFT);
    App_SetButtonManually(
        &Input.keyboard.rightKey.pressed, 
        &Input.keyboard.rightKey.released, 
        &Input.keyboard.rightKey.held,
        VK_RIGHT);
    App_SetButtonManually(
        &Input.keyboard.wKey.pressed, 
        &Input.keyboard.wKey.released, 
        &Input.keyboard.wKey.held, 
        'W');
    App_SetButtonManually(
        &Input.keyboard.sKey.pressed, 
        &Input.keyboard.sKey.released, 
        &Input.keyboard.sKey.held,
        'S');
    App_SetButtonManually(
        &Input.keyboard.aKey.pressed, 
        &Input.keyboard.aKey.released, 
        &Input.keyboard.aKey.held,
        'A');
    App_SetButtonManually(
        &Input.keyboard.dKey.pressed, 
        &Input.keyboard.dKey.released, 
        &Input.keyboard.dKey.held,
        'D');

    App_SetButtonManually(
        &Input.keyboard.qKey.pressed, 
        &Input.keyboard.qKey.released, 
        &Input.keyboard.qKey.held, 
        'Q');
    App_SetButtonManually(
        &Input.keyboard.eKey.pressed, 
        &Input.keyboard.eKey.released, 
        &Input.keyboard.eKey.held, 
        'E');
    App_SetButtonManually(
        &Input.keyboard.rKey.pressed, 
        &Input.keyboard.rKey.released, 
        &Input.keyboard.rKey.held, 
        'R');
    App_SetButtonManually(
        &Input.keyboard.tKey.pressed, 
        &Input.keyboard.tKey.released, 
        &Input.keyboard.tKey.held, 
        'T');
    App_SetButtonManually(
        &Input.keyboard.yKey.pressed, 
        &Input.keyboard.yKey.released, 
        &Input.keyboard.yKey.held, 
        'Y');
    App_SetButtonManually(
        &Input.keyboard.uKey.pressed, 
        &Input.keyboard.uKey.released, 
        &Input.keyboard.uKey.held, 
        'U');
    App_SetButtonManually(
        &Input.keyboard.iKey.pressed, 
        &Input.keyboard.iKey.released, 
        &Input.keyboard.iKey.held, 
        'I');
    App_SetButtonManually(
        &Input.keyboard.oKey.pressed, 
        &Input.keyboard.oKey.released, 
        &Input.keyboard.oKey.held, 
        'O');
    App_SetButtonManually(
        &Input.keyboard.pKey.pressed, 
        &Input.keyboard.pKey.released, 
        &Input.keyboard.pKey.held, 
        'P');

    App_SetButtonManually(
        &Input.keyboard.fKey.pressed, 
        &Input.keyboard.fKey.released, 
        &Input.keyboard.fKey.held, 
        'F');
    App_SetButtonManually(
        &Input.keyboard.gKey.pressed, 
        &Input.keyboard.gKey.released, 
        &Input.keyboard.gKey.held, 
        'G');
    App_SetButtonManually(
        &Input.keyboard.hKey.pressed, 
        &Input.keyboard.hKey.released, 
        &Input.keyboard.hKey.held, 
        'H');
    App_SetButtonManually(
        &Input.keyboard.jKey.pressed, 
        &Input.keyboard.jKey.released, 
        &Input.keyboard.jKey.held,
        'J');
    App_SetButtonManually(
        &Input.keyboard.kKey.pressed, 
        &Input.keyboard.kKey.released, 
        &Input.keyboard.kKey.held,
        'K');
    App_SetButtonManually(
        &Input.keyboard.lKey.pressed, 
        &Input.keyboard.lKey.released, 
        &Input.keyboard.lKey.held,
        'L');

    App_SetButtonManually(
        &Input.keyboard.zKey.pressed, 
        &Input.keyboard.zKey.released, 
        &Input.keyboard.zKey.held,
        'Z');
    App_SetButtonManually(
        &Input.keyboard.xKey.pressed, 
        &Input.keyboard.xKey.released, 
        &Input.keyboard.xKey.held,
        'X');
    App_SetButtonManually(
        &Input.keyboard.cKey.pressed, 
        &Input.keyboard.cKey.released, 
        &Input.keyboard.cKey.held,
        'C');
    App_SetButtonManually(
        &Input.keyboard.vKey.pressed, 
        &Input.keyboard.vKey.released, 
        &Input.keyboard.vKey.held,
        'V');
    App_SetButtonManually(
        &Input.keyboard.bKey.pressed, 
        &Input.keyboard.bKey.released, 
        &Input.keyboard.bKey.held,
        'B');
    App_SetButtonManually(
        &Input.keyboard.nKey.pressed, 
        &Input.keyboard.nKey.released, 
        &Input.keyboard.nKey.held,
        'N');
    App_SetButtonManually(
        &Input.keyboard.mKey.pressed, 
        &Input.keyboard.mKey.released, 
        &Input.keyboard.mKey.held,
        'M');

    App_SetButtonManually(
        &Input.keyboard.spaceBar.pressed, 
        &Input.keyboard.spaceBar.released, 
        &Input.keyboard.spaceBar.held,
        VK_SPACE);
    App_SetButtonManually(
        &Input.keyboard.tabKey.pressed, 
        &Input.keyboard.tabKey.released, 
        &Input.keyboard.tabKey.held,
        VK_TAB);
    App_SetButtonManually(
        &Input.keyboard.enterKey.pressed, 
        &Input.keyboard.enterKey.released, 
        &Input.keyboard.enterKey.held,
        VK_RETURN);
    App_SetButtonManually(
        &Input.keyboard.escapeKey.pressed, 
        &Input.keyboard.escapeKey.released, 
        &Input.keyboard.escapeKey.held,
        VK_ESCAPE);
    App_SetButtonManually(
        &Input.keyboard.leftShiftKey.pressed, 
        &Input.keyboard.leftShiftKey.released, 
        &Input.keyboard.leftShiftKey.held,
        VK_LSHIFT);
    App_SetButtonManually(
        &Input.keyboard.rightShiftKey.pressed, 
        &Input.keyboard.rightShiftKey.released, 
        &Input.keyboard.rightShiftKey.held,
        VK_RSHIFT);
    Input.keyboard.shiftKey.pressed = Input.keyboard.leftShiftKey.pressed || Input.keyboard.rightShiftKey.pressed;
    Input.keyboard.shiftKey.released = Input.keyboard.leftShiftKey.released || Input.keyboard.rightShiftKey.released;
    Input.keyboard.shiftKey.held = Input.keyboard.leftShiftKey.held || Input.keyboard.rightShiftKey.held;

#pragma endregion

#pragma region Number Keys

    App_SetButtonManually(
        &Input.keyboard.numRow1.pressed,
        &Input.keyboard.numRow1.held,
        &Input.keyboard.numRow1.released,
        VK_1KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow2.pressed,
        &Input.keyboard.numRow2.held,
        &Input.keyboard.numRow2.released,
        VK_2KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow3.pressed,
        &Input.keyboard.numRow3.held,
        &Input.keyboard.numRow3.released,
        VK_3KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow4.pressed,
        &Input.keyboard.numRow4.held,
        &Input.keyboard.numRow4.released,
        VK_4KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow5.pressed,
        &Input.keyboard.numRow5.held,
        &Input.keyboard.numRow5.released,
        VK_5KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow6.pressed,
        &Input.keyboard.numRow6.held,
        &Input.keyboard.numRow6.released,
        VK_6KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow7.pressed,
        &Input.keyboard.numRow7.held,
        &Input.keyboard.numRow7.released,
        VK_7KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow8.pressed,
        &Input.keyboard.numRow8.held,
        &Input.keyboard.numRow8.released,
        VK_8KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow9.pressed,
        &Input.keyboard.numRow9.held,
        &Input.keyboard.numRow9.released,
        VK_9KEY);
    App_SetButtonManually(
        &Input.keyboard.numRow0.pressed,
        &Input.keyboard.numRow0.held,
        &Input.keyboard.numRow0,
        VK_0KEY);

    App_SetButtonManually(
        &Input.keyboard.numPad1.pressed,
        &Input.keyboard.numPad1.held,
        &Input.keyboard.numPad1.released,
        VK_NUMPAD1);
    App_SetButtonManually(
        &Input.keyboard.numPad2.pressed,
        &Input.keyboard.numPad2.held,
        &Input.keyboard.numPad2.released,
        VK_NUMPAD2);
    App_SetButtonManually(
        &Input.keyboard.numPad3.pressed,
        &Input.keyboard.numPad3.held,
        &Input.keyboard.numPad3.released,
        VK_NUMPAD3);
    App_SetButtonManually(
        &Input.keyboard.numPad4.pressed,
        &Input.keyboard.numPad4.held,
        &Input.keyboard.numPad4.released,
        VK_NUMPAD4);
    App_SetButtonManually(
        &Input.keyboard.numPad5.pressed,
        &Input.keyboard.numPad5.held,
        &Input.keyboard.numPad5.released,
        VK_NUMPAD5);
    App_SetButtonManually(
        &Input.keyboard.numPad6.pressed,
        &Input.keyboard.numPad6.held,
        &Input.keyboard.numPad6.released,
        VK_NUMPAD6);
    App_SetButtonManually(
        &Input.keyboard.numPad7.pressed,
        &Input.keyboard.numPad7.held,
        &Input.keyboard.numPad7.released,
        VK_NUMPAD7);
    App_SetButtonManually(
        &Input.keyboard.numPad8.pressed,
        &Input.keyboard.numPad8.held,
        &Input.keyboard.numPad8.released,
        VK_NUMPAD8);
    App_SetButtonManually(
        &Input.keyboard.numPad9.pressed,
        &Input.keyboard.numPad9.held,
        &Input.keyboard.numPad9.released,
        VK_NUMPAD9);
    App_SetButtonManually(
        &Input.keyboard.numPad0.pressed,
        &Input.keyboard.numPad0.held,
        &Input.keyboard.numPad0.released,
        VK_NUMPAD0);

    App_SetDualButton(
        &Input.keyboard.number1,
        &Input.keyboard.numPad1,
        &Input.keyboard.numRow1);
    App_SetDualButton(
        &Input.keyboard.number2,
        &Input.keyboard.numPad2,
        &Input.keyboard.numRow2);
    App_SetDualButton(
        &Input.keyboard.number3,
        &Input.keyboard.numPad3,
        &Input.keyboard.numRow3);
    App_SetDualButton(
        &Input.keyboard.number4,
        &Input.keyboard.numPad4,
        &Input.keyboard.numRow4);
    App_SetDualButton(
        &Input.keyboard.number5,
        &Input.keyboard.numPad5,
        &Input.keyboard.numRow5);
    App_SetDualButton(
        &Input.keyboard.number6,
        &Input.keyboard.numPad6,
        &Input.keyboard.numRow6);
    App_SetDualButton(
        &Input.keyboard.number7,
        &Input.keyboard.numPad7,
        &Input.keyboard.numRow7);
    App_SetDualButton(
        &Input.keyboard.number8,
        &Input.keyboard.numPad8,
        &Input.keyboard.numRow8);
    App_SetDualButton(
        &Input.keyboard.number9,
        &Input.keyboard.numPad9,
        &Input.keyboard.numRow9);
    App_SetDualButton(
        &Input.keyboard.number0,
        &Input.keyboard.numPad0,
        &Input.keyboard.numRow0);



#pragma endregion


    short debugKeyIsPressed = Input.keyboard.tabKey.pressed;
    //static short debugKeyIsHeld;//Stays between calls

    short shiftKeyIsPressed = Input.keyboard.shiftKey.held;
    
    short escapeKeyIsPressed = Input.keyboard.escapeKey.pressed;
    short upInputIsPressed = Input.keyboard.wKey.held | Input.keyboard.upKey.held;
    short leftInputIsPressed = Input.keyboard.aKey.held | Input.keyboard.leftKey.held;
    short downInputIsPressed = Input.keyboard.sKey.held | Input.keyboard.downKey.held;
    short rightInputIsPressed = Input.keyboard.dKey.held | Input.keyboard.rightKey.held;
    short actionButtonIsPressed = Input.keyboard.spaceBar.pressed | Input.keyboard.zKey.pressed | Input.keyboard.jKey.pressed;
    static short actionButtonIsHeld;

    int hori = 0;
    int vert = 0;
    static int dirX = 0;
    static int dirY = -1;
    Transform* trans_ = (Transform*)GetComponent(DSID_Transform, &player);
    if (trans_ == NULL)
        return;
    Animator* anim_ = (Animator*)GetComponent(DSID_Animator, &player);

    if (Input.keyboard.oKey.held)
        trans_->positionZ--;
    if (Input.keyboard.pKey.held)
        trans_->positionZ++;

    if (debugKeyIsPressed)// && !debugKeyIsHeld)
        gDebug.showDebugText ^= TRUE;

    if (escapeKeyIsPressed!= 0)
        SendMessage(gMainWindow, WM_CLOSE, 0, 0);

    if (shiftKeyIsPressed)
    {
        player.speed = player.baseSpeed * 1.8f;
        //player.info.disabled = true;
    }
    else
    {
        player.speed = player.baseSpeed;
        //player.info.disabled = false;
    }

    if (leftInputIsPressed)
    {
        hori = -1;
        dirX = hori;
    }
    if (rightInputIsPressed)
    {
        hori = 1;
        dirX = hori;
    }
    if ((leftInputIsPressed && rightInputIsPressed) || (!leftInputIsPressed && !rightInputIsPressed))
        hori = 0;

    if (downInputIsPressed)
    {
        vert = -1;
        dirY = vert;
    }
    if (upInputIsPressed)
    {
        vert = 1;
        dirY = vert;
    }
    if ((downInputIsPressed && upInputIsPressed) || (!downInputIsPressed && !upInputIsPressed))
        vert = 0;

    //One dir at a time
    if (vert) dirX = 0;
    else if (hori) dirY = 0;
    
    //Player logic
    Transform_Translate(trans_, hori * player.speed * gTime.deltaTime, vert * player.speed * gTime.deltaTime);
    if (hori)
    {
        Animator_Play(anim_, "Walking", ClampInt(dirX, ANIM_DOWN, ANIM_UP) + 2, true, 0, false);
    }
    if (vert)
    {
        Animator_Play(anim_, "Walking", ClampInt(vert, ANIM_DOWN, ANIM_UP), true, 0, false);
    }

    if (!hori && !vert)
    {
        if (dirX)
        {
            Animator_Play(anim_, "Idle", ClampInt(dirX, ANIM_DOWN, ANIM_UP) + 2, true, 0, false);
            //if (Animator_Play(anim_, "Idle", ANIM_RIGHT, true, 0, false) == true)
            //{
            //    if (dirX < 0)//left
            //        trans_->scaleX = -1;
            //    else
            //        trans_->scaleX = 1;
            //}
        }
        else if (dirY)
        {
            Animator_Play(anim_, "Idle", ClampInt(dirY, ANIM_DOWN, ANIM_UP), true, 0, false);
        }
    }

    static bool secondAttack = false;
    if (actionButtonIsPressed && !actionButtonIsHeld)
    {
        bool justAttacked = false;
        if (dirX)
        {
            if (!secondAttack)
                justAttacked = Animator_Play(anim_, "Attack", ClampInt(dirX, ANIM_DOWN, ANIM_UP) + 2, false, 1, false) == DSMSG_ANIM_PLAYED;
            else
            {
                justAttacked = Animator_Play(anim_, "Attack", WrapInt(dirX, ANIM_DOWN, ANIM_UP) + 2, false, 1, false) == DSMSG_ANIM_PLAYED;
                if (justAttacked)
                    anim_->flipFrames = true;
            }
        }
        else if (dirY)
        {
            if (!secondAttack)
                justAttacked = Animator_Play(anim_, "Attack", ClampInt(dirY, ANIM_DOWN, ANIM_UP), false, 1, false) == DSMSG_ANIM_PLAYED;
            else
            {
                justAttacked = Animator_Play(anim_, "Attack", ClampInt(dirY, ANIM_DOWN, ANIM_UP), false, 1, false) == DSMSG_ANIM_PLAYED;
                if (justAttacked)
                    anim_->flipFrames = true;
            }
        }

        if (justAttacked)
            secondAttack = !secondAttack;
        //trans_->scaleX = 1;
    }

    //
    //MOUSEINPUT mi = { 0 };
    //mouse_event(mi.dwFlags, mi.dx, mi.dy, mi.mouseData, mi.dwExtraInfo);
    
    //trans_->positionX = Input.mouse.pixelPosition.x;//5?
    //trans_->positionY = Input.mouse.pixelPosition.y;//4.488f?
    
    //Set static vars
    //debugKeyIsHeld = debugKeyIsPressed;

    //actionButtonIsHeld = actionButtonIsPressed;
}
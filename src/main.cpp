#include <windows.h>

#include <cstdio>
#include <cstdint>
#include <cmath>

#include <glad/gl.h>
// #include <GL/gl.h>
#include "wglext.h"
// #include "glext.h"

static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 0;

#define L_CHECK_WGL_PROC_ADDR(identifier) {\
    void *pfn = (void*)identifier;\
    if ((pfn == 0)\
    ||  (pfn == (void*)0x1) || (pfn == (void*)0x2) || (pfn == (void*)0x3)\
    ||  (pfn == (void*)-1)\
    ) return procedureIndex;\
    ++procedureIndex;\
}

int loadWGLFunctionPointers()
{
    int procedureIndex = 0;

    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    L_CHECK_WGL_PROC_ADDR(wglCreateContextAttribsARB);

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
        wglGetProcAddress("wglSwapIntervalEXT");
    L_CHECK_WGL_PROC_ADDR(wglSwapIntervalEXT);

    return -1;
}

static FILE *debugStream = nullptr;

void logOut(const char *message, const char *content = NULL)
{
    fputs(message, debugStream);
    fputs(content, debugStream);
    fputs("\n", debugStream);
}

static const char *windowTitle = "Opengl Test";

static bool globalRunning = true;
static int clientWidth = 1080;
static int clientHeight = 720;

static HWND window = nullptr;

void errorMessage(const char *message)
{
    MessageBoxA(NULL, message, windowTitle, MB_ICONERROR);
}

static bool openglLoaded = false;

LRESULT CALLBACK WindowProc(
        HWND windowHandle,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
);

int WinMain(
        HINSTANCE instance,
        HINSTANCE previousInstance,
        LPSTR commandLine,
        int nShowCmd
) {
    debugStream = fopen("debug_log.txt", "w");

    WNDCLASSA windowClass = {};
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = &WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "windozer.";

    ATOM registerOutput = RegisterClassA(&windowClass);
    if (!registerOutput)
    {
        errorMessage("Class registration failed!");
    }

    window = CreateWindowExA(
        0,
        windowClass.lpszClassName,
        "minecraft.exe",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        clientWidth,
        clientHeight,
        0,
        0,
        instance,
        0
    );

    if (!window)
    {
        errorMessage("Window creation failed!");
    }

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE; // NOTE: Might not be necessary

    HDC retrievedDC = GetDC(window);
    if (!retrievedDC)
    {
        errorMessage("Failed to retrieve device context for ChoosePixelFormat!");
    }

    int pfIndex = ChoosePixelFormat(retrievedDC, &pfd);
    if (pfIndex == 0)
    {
        errorMessage("Failed to choose pixel format!");
    }

    BOOL bResult = SetPixelFormat(retrievedDC, pfIndex, &pfd);
    if (!bResult)
    {
        errorMessage("Failed to set pixel format!");
    }

    HGLRC tContext = wglCreateContext(retrievedDC);
    if (!tContext)
    {
        errorMessage("Failed to create temporary OpenGL context!");
    }

    bResult = wglMakeCurrent(retrievedDC, tContext);
    if (!bResult)
    {
        errorMessage("Failed to make temporary OpenGL context current!");
    }

    int procIndex = -1;
    if ((procIndex = loadWGLFunctionPointers()) != -1)
    {
        fprintf(debugStream, "Failed to retrieve procedure number %d!", procIndex);
        fflush(debugStream);
        errorMessage("Failed to load WGL extension functions!");
    }

    int attribs[]
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 4,
        WGL_CONTEXT_FLAGS_ARB, 0,
        0
    };

    HGLRC openglContext = wglCreateContextAttribsARB(retrievedDC, 0, attribs);
    if (!openglContext)
    {
        errorMessage("Failed to create OpenGl context!");
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tContext);
    wglMakeCurrent(retrievedDC, openglContext);

    ReleaseDC(window, retrievedDC);

    if (!gladLoaderLoadGL())
    {
        errorMessage("glad failed to load OpenGL!");
    }
    openglLoaded = true;

    const char *vendorString = (const char*)glGetString(GL_VENDOR);
    const char *rendererString = (const char*)glGetString(GL_RENDERER);
    const char *versionString = (const char*)glGetString(GL_VERSION);
    const char *slVersionString = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    logOut("Vendor: ", vendorString);
    logOut("Renderer: ", rendererString);
    logOut("Version: ", versionString);
    logOut("Shading Language Version: ", slVersionString);
    fflush(debugStream);

//     logOut("Extensions");
//     int extCount;
//     glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
//     for (int i = 0; i < extCount; i++)
//     {
//         logOut("\t- ", (const char*)glGetStringi(GL_EXTENSIONS, i));
//     }
    
//     for (int i = 0; i < 256; i++)
//     {
//         fprintf(debugStream, "%d: %c\n", i, i);
//     }

    wglSwapIntervalEXT(0);

    uint64_t frameCount = 0;

    while (globalRunning)
    {
        MSG message;
        while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                globalRunning = false;
            }

            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        float brightness = (float)sin((double)frameCount / 64.0);
        glClearColor(brightness, brightness, brightness, 1.0f);
        ++frameCount;

        HDC retrievedDC = GetDC(window);
        SwapBuffers(retrievedDC);
        ReleaseDC(window, retrievedDC);
    }

    wglDeleteContext(openglContext);

    fclose(debugStream);

    return 0;
}

LRESULT CALLBACK WindowProc(
        HWND windowHandle,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
) {
    LRESULT result = 0;

    switch (message)
    {
        case WM_SIZE:
        {
            if (!openglLoaded)
            {
                break;
            }

            RECT currentRectangle;
            GetClientRect(window, &currentRectangle);
            int rectangleWidth = currentRectangle.right - currentRectangle.left;
            int rectangleHeight = currentRectangle.bottom - currentRectangle.top;

            glViewport(0, 0, rectangleWidth, rectangleHeight);
        } break;

        case WM_CLOSE:
        {
            globalRunning = false;
        } break;

        case WM_KEYDOWN:
        {
            fprintf(debugStream, "WM_KEYDOWN: %c, %llx\n", wParam, wParam);
        } break;

        case WM_CHAR:
        {
            fprintf(debugStream, "WM_CHAR: %c, %llx\n", wParam, wParam);
        } break;

        default:
        {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        }
    }

    return result;
}

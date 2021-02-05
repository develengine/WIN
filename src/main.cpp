#include <windows.h>
#include <locale.h>
#include <wchar.h>

#include <cstdio>
#include <cstdint>
#include <cmath>

#include <glad/gl.h>
// #include <GL/gl.h>
#include "wglext.h"
// #include "glext.h"

// FIXME Can't change input method while window selected.

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "math.hpp"

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

float cubeVertices[]
{
     1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
     1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f,

    -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f,-1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f,-1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f,-1.0f,-1.0f, -1.0f, 0.0f, 0.0f,

    -1.0f, 1.0f,-1.0f,  0.0f, 0.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,  0.0f, 0.0f,-1.0f,
     1.0f, 1.0f,-1.0f,  0.0f, 0.0f,-1.0f,
     1.0f,-1.0f,-1.0f,  0.0f, 0.0f,-1.0f,

     1.0f, 1.0f,-1.0f,  1.0f, 0.0f, 0.0f,
     1.0f,-1.0f,-1.0f,  1.0f, 0.0f, 0.0f,
     1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
     1.0f,-1.0f, 1.0f,  1.0f, 0.0f, 0.0f,

     1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
     1.0f, 1.0f,-1.0f,  0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f,-1.0f,  0.0f, 1.0f, 0.0f,

     1.0f,-1.0f,-1.0f,  0.0f,-1.0f, 0.0f,
    -1.0f,-1.0f,-1.0f,  0.0f,-1.0f, 0.0f,
     1.0f,-1.0f, 1.0f,  0.0f,-1.0f, 0.0f,
    -1.0f,-1.0f, 1.0f,  0.0f,-1.0f, 0.0f
};

uint32_t cubeIndices[]
{
    0, 2, 3,  0, 3, 1,
    4, 6, 7,  4, 7, 5,
    8, 10, 11,  8, 11, 9,
    12, 14, 15,  12, 15, 13,
    16, 18, 19,  16, 19, 17,
    20, 22, 23,  20, 23, 21
};

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

void GLAPIENTRY openglCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
) {
    bool error = type == GL_DEBUG_TYPE_ERROR;
//     std::cerr << "GL: error: " << error << " type: " << type << " severity: " << severity
//               << ".\nmessage: " << message << '\n';
    fprintf(debugStream, "GL: error: %d, type: %d, severity: %d\nmessage: %s\n",
            error, type, severity, message);
}

static const char *windowTitle = "Opengl Test";

static bool globalRunning = true;
static int clientWidth = 1080;
static int clientHeight = 720;

static HWND window = nullptr;

static int lastMouseX = -1;
static int lastMouseY = -1;

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

char *readFileToString(const char *path)
{
    FILE *file = fopen(path, "r");
    
    if (!file)
    {
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);
    
    char *data = new char[size];
    fread(data, size, 1, file);
    fclose(file);

    data[size - 1] = '\0';

    return data;
}

#define DEBUG_CONSOLE
#ifdef DEBUG_CONSOLE
int main(int argc, char *argv[]) {
    HINSTANCE instance = GetModuleHandle(NULL);
#else
int WinMain(
        HINSTANCE instance,
        HINSTANCE previousInstance,
        LPSTR commandLine,
        int nShowCmd
) {
#endif
    debugStream = fopen("debug_log.txt", "w");

    WNDCLASSW windowClass = {};
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = &WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = L"windozer.";

    ATOM registerOutput = RegisterClassW(&windowClass);
    if (!registerOutput)
    {
        errorMessage("Class registration failed!");
    }

    window = CreateWindowExW(
        0,
        windowClass.lpszClassName,
        L"minecraft.exe",
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

    printf("Is window UNICODE: %d\n", IsWindowUnicode(window));

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

    setlocale(LC_ALL, "");

    // Raw Gachi Input

    RAWINPUTDEVICE rawInputDevice;
    rawInputDevice.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
    rawInputDevice.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
    rawInputDevice.dwFlags = 0;
    rawInputDevice.hwndTarget = window;

    if (!RegisterRawInputDevices(&rawInputDevice, 1, sizeof(rawInputDevice)))
    {
        errorMessage("Failed to 'RegisterRawInputDevices'!");
    }


    // Opengl

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(openglCallback, 0); 

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

    wglSwapIntervalEXT(1);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
//     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    uint32_t vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    uint32_t ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    int succes;
    char infoLog[512];

    const char *vertexSource = readFileToString("shaders/cringe.vert");
    const char *fragmentSource = readFileToString("shaders/cringe.frag");

    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &succes);

    if (!succes)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
//         std::cerr << "Failed to compile vertex shader! Error:\n" << infoLog << '\n';
        fprintf(debugStream, "Failed to compile vertex shader! Error:%s\n", infoLog);
    }
    
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &succes);

    if (!succes)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
//         std::cerr << "Failed to compile fragment shader! Error:\n" << infoLog << '\n';
        fprintf(debugStream, "Failed to compile fragment shader! Error:%s\n", infoLog);
    }
    
    uint32_t program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &succes);

    if (!succes)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Failed to link shader program! Error:\n" << infoLog << '\n';
    }

    delete[] vertexSource;
    delete[] fragmentSource;

    int uVpMatrix = glGetUniformLocation(program, "u_vpMat");
    int uModelMatrix = glGetUniformLocation(program, "u_modMat");
    int uCameraPosition = glGetUniformLocation(program, "u_cameraPos");
    int uObjectColor = glGetUniformLocation(program, "u_objColor");

    eng::Vec3f cameraPosition(0.0f, 0.0f, 0.0f);
    eng::Vec2f cameraRotation(0.0f, 0.0f);

    eng::Vec3f objectPosition(0.0f, 0.0f, -10.0f);
    eng::Vec3f objectAxis = eng::Vec3f(1.0f, 1.0f, 0.0f).normalize();

    float timePassed = 0.0f;


    uint64_t frameCount = 0;

    // NOTE: Might needto be in the loop
    HDC retrievedDCSwap = GetDC(window);

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

//         glClear(GL_COLOR_BUFFER_BIT);
//         float brightness = (float)sin((double)frameCount / 64.0);
//         glClearColor(brightness, brightness, brightness, 1.0f);
//         ++frameCount;

        eng::Quaternionf objectRotation(cos(timePassed), (objectAxis * sin(timePassed)).data);
        timePassed += 0.009f;
        eng::Mat4f modelMatrix = eng::Mat4f::translation(objectPosition.data)
                               * eng::Mat4f::rotation(objectRotation.normalize());
        eng::Mat4f viewMatrix = eng::Mat4f::xRotation(cameraRotation[0])
                              * eng::Mat4f::yRotation(cameraRotation[1])
                              * eng::Mat4f::translation((-cameraPosition).data);
        eng::Mat4f projectionMatrix = eng::Mat4f::GL_Projection(90.f, clientWidth, clientHeight, 0.1f, 100.f);
        eng::Mat4f vpMatrix = projectionMatrix * viewMatrix;

        glClearColor(5.0f, 7.0f, 9.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vao);
        glUseProgram(program);

        glUniformMatrix4fv(uVpMatrix, 1, false, vpMatrix.data);
        glUniformMatrix4fv(uModelMatrix, 1, false, modelMatrix.data);
        glUniform3fv(uCameraPosition, 1, cameraPosition.data);
        glUniform3f(uObjectColor, 0.8f, 0.7f, 0.45f);

        glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(uint32_t), GL_UNSIGNED_INT, (void*)(0));

        glBindVertexArray(0);
        glUseProgram(0);

        SwapBuffers(retrievedDCSwap);
    }

    ReleaseDC(window, retrievedDCSwap);

    wglDeleteContext(openglContext);

    fclose(debugStream);

    return 0;
}


WINDOWPLACEMENT previousWP = { sizeof(previousWP) };

void goFullscreen(HWND hwnd) {
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);

    if (style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };

        if (GetWindowPlacement(hwnd, &previousWP) &&
                GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY),
                    &monitorInfo)
        ) {
            SetWindowLong(hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP,
                    monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                    monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                    monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &previousWP);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}


int mouseRelative = 0;

void updateCursorRect(HWND hwnd) {
    if (mouseRelative) {
        RECT windowRect;
        GetClientRect(hwnd, &windowRect);
        ClientToScreen(hwnd, (POINT*)&windowRect.left);
        ClientToScreen(hwnd, (POINT*)&windowRect.right);
        ClipCursor(&windowRect);
    }
}

void relativeMouse(HWND hwnd, int val) {
    if (val) {
        if (!mouseRelative) {
            mouseRelative = 1;
            updateCursorRect(hwnd);
            SetCursor(NULL);
        }
    } else {
        if (mouseRelative) {
            mouseRelative = 0;
            ClipCursor(NULL);
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }
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

            clientWidth = rectangleWidth;
            clientHeight = rectangleHeight;

            updateCursorRect(windowHandle);
        } break;

        case WM_CLOSE:
        {
            globalRunning = false;
        } break;

        case WM_INPUT:
        {
            uint32_t dataSize = 0;
            HRAWINPUT rawInput = (HRAWINPUT)lParam;
            RAWINPUT *data = NULL;

            GetRawInputData(rawInput, RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
            uint8_t *rawData = new uint8_t[dataSize];
            if (GetRawInputData(rawInput, RID_INPUT, rawData, &dataSize, sizeof(RAWINPUTHEADER)) == (uint32_t)-1)
            {
                errorMessage("Failedto retrieve raw input data!");
            }
            data = (RAWINPUT*)rawData;

            // int dx, dy;
            bool absoluteMode = (data->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE;
            // if ((data->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
            // {
                // 
            // }
            if (mouseRelative) {
                printf("%s: %d, %d\n",
                        absoluteMode ? "ABS" : "REL",
                        data->data.mouse.lLastX,
                        data->data.mouse.lLastY);
            }

            delete[] rawData;
        } break;

        case WM_KEYDOWN:
        {
            fprintf(debugStream, "WM_KEYDOWN: %c, %llx\n", int(wParam), wParam);
        } break;

        case WM_CHAR:
        {
            if ((unsigned int)wParam >= 0xD800 && (unsigned int)wParam <= 0xDBFF) {
                printf("surrogate\n");
            }

            if ((char)wParam == 'f') {
                goFullscreen(windowHandle);
            } else if ((char)wParam == 'm') {
                relativeMouse(windowHandle, !mouseRelative);
            }

            printf("WM_CHAR: %lc, %llx\n", (unsigned int)wParam, wParam);
        } break;

        default:
        {
            result = DefWindowProcW(windowHandle, message, wParam, lParam);
        }
    }

    return result;
}

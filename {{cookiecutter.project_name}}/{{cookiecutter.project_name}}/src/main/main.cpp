#include "src/main/constants.hpp"
#include <cstdio>
#include <d2d1.h>
#include <iostream>
#include <vcruntime.h>
#include <windows.h>
#include <windowsx.h>
#include <winnt.h>
#pragma comment(lib, "d2d1")

#include "app.hpp"
#include "util.hpp"

LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND makeWindow(
    const wchar_t* className,
    const wchar_t* title,
    WNDPROC windowProc,
    HINSTANCE hInstance,
    int x,
    int y,
    int w,
    int h,
    void* param
);

int main() {
    wWinMain(GetModuleHandle(nullptr), nullptr, nullptr, 1);
    return 0;
}

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PWSTR pCmdLine,
    int nCmdShow
) {
    // show debug console always:
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
    freopen_s(&stream, "CONIN$", "r", stdin);

    HWND window = makeWindow(
        L"My Class",
        L"My Title",
        windowProc,
        hInstance,
        CW_USEDEFAULT,   // window x position
        CW_USEDEFAULT,   // window y position
        windowWidth,
        windowHeight,
        nullptr
    );

    if (window == nullptr) {
        return 1;
    }

    ShowWindow(window, nCmdShow);

    MSG message = {};
    while (message.message != WM_QUIT) {
        if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        } else {
            App* app = (App*)getInstanceData(window);
            if (app != nullptr) {
                app->tick();
            }
            Sleep(10);
        }
    }

    return (int)message.wParam;
}

HWND makeWindow(
    const wchar_t* className,
    const wchar_t* title,
    WNDPROC windowProc,
    HINSTANCE hInstance,
    int x,
    int y,
    int w,
    int h,
    void* param
) {
    HWND window = nullptr;

    WNDCLASSEX windowClass = {0};

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    ATOM registerClassRes = RegisterClassExW(&windowClass);
    if (!registerClassRes) {
        messageBox(L"RegisterClassEx() failed");
        return window;
    }

    // "adjust" the window size to actually match `w` and `h`
    // the window size will actually still be 5 pixels less than `w` and `h`
    // for example, if `w` and `h` are both 1000, the actual window size will be 995 x 995

    SetProcessDPIAware();

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + w;
    rect.bottom = y + h;

    BOOL menu = FALSE;  // Do you have a menu?
                        // In this case, a "menu" means a top menu with options like "File", "Edit", etc.

    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    AdjustWindowRectEx(&rect, dwStyle, menu, 0);

    int adjustedW = rect.right - rect.left;
    int adjustedH = rect.bottom - rect.top;

    window = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW,
        className,
        title,
        dwStyle,                // window style
        x,
        y,
        adjustedW,
        adjustedH,
        nullptr,
        nullptr,
        hInstance,
        param
    );

    if (!window) {
        messageBox(L"CreateWindow() failed");
        return window;
    }

    return window;
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    HRESULT hr;
    LRESULT lr = 0;

    if (message == WM_CREATE) {
        HRESULT hr;
        App* app = new App(window, hr);
        if (FAILED(hr)) {
            messageBoxError(hr);
            DestroyWindow(window);
        }

        storeInstanceData(window, app);
    } else if (message == WM_DESTROY) {
        App* app = (App*)getInstanceData(window);
        app->destroy();
        PostQuitMessage(0);
    } else {
        App* app = (App*)getInstanceData(window);
        if (app != nullptr && app->shouldHandleMessage(message)) {
            hr = app->handleMessage(message, wParam, lParam);
            if (FAILED(hr)) {
                messageBoxError(hr);
                DestroyWindow(window);
            }
        } else {
            lr = DefWindowProcW(window, message, wParam, lParam);
        }
    }

    return lr;
}

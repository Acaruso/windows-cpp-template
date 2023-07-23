#include <cstdio>
#include <d2d1.h>
#include <iostream>
#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "d2d1")

#include "app.hpp"
#include "util.hpp"

int main() {
    wWinMain(GetModuleHandle(NULL), NULL, NULL, 1); 
    // system("pause");
    return 0;
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
    std::cout << "Hello World" << std::endl;
    HWND window = NULL;

    WNDCLASSEX windowClass = {0};

    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);

    ATOM registerClassRes = RegisterClassExW(&windowClass);
    if (!registerClassRes) {
        messageBox(L"RegisterClassEx() failed");
        return window;
    }

    window = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW,
        className,
        title,
        WS_OVERLAPPEDWINDOW,
        x,
        y,
        w,
        h,
        NULL,
        NULL,
        hInstance,
        param
    );

    if (!window) {
        messageBox(L"CreateWindow() failed");
        return window;
    }

    return window;
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PWSTR pCmdLine,
    int nCmdShow
) {
    App app;

    HWND window = makeWindow(
        L"My Class",
        L"My Title",
        windowProc,
        hInstance,
        CW_USEDEFAULT,  // x
        CW_USEDEFAULT,  // y
        500,            // width
        500,            // height
        &app
    );

    if (window == NULL) {
        return 1;
    }

    ShowWindow(window, nCmdShow);

    MSG message = {};
    while (message.message != WM_QUIT) {
        if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        } else {
            app.tick();
            Sleep(10);
        }
    }

    return (int)message.wParam;
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    HRESULT hr;
    LRESULT lr = 0;

    if (message == WM_CREATE) {
        storeInstanceData(window, lParam);
        App* app = (App*)getInstanceData(window);
        
        hr = app->init(window);
        if (FAILED(hr)) {
            messageBoxError(hr);
            DestroyWindow(window);
        }
    } else if (message == WM_DESTROY) {
        App* pApp = (App*)getInstanceData(window);
        pApp->destroy();
        PostQuitMessage(0);
    } else {
        App* app = (App*)getInstanceData(window);
        if (app != NULL && app->shouldHandleMessage(message)) {
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

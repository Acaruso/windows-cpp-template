#pragma once

#include <cstdio>
#include <thread>
#include <vector>

#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include "../lib/readerwriterqueue.h"

#include "../audio/audio_main.hpp"
#include "constants.hpp"
#include "graphics_service.hpp"
#include "util.hpp"

class App {
public:
    HWND window;
    GraphicsService gfx;
    moodycamel::ReaderWriterQueue<std::string> queue;
    std::thread audioThread;

    std::vector<UINT> messageTypes{
        WM_PAINT,
        WM_LBUTTONDOWN,
        WM_MOUSEMOVE
    };

    D2D1_RECT_F rect{100, 100, 150, 150};

    HRESULT init(HWND window) {
        HRESULT hr;
        this->window = window;
        hr = gfx.init(window);
        audioThread = std::thread(&audioMain, &queue);
        return hr;
    }

    bool shouldHandleMessage(UINT message) {
        for (auto elt : messageTypes) {
            if (message == elt) {
                return true;
            }
        }
        return false;
    }

    HRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        HRESULT hr = S_OK;

        if (message == WM_PAINT) {
            hr = onPaint();
        } else if (message == WM_LBUTTONDOWN) {
            onLeftClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        } else if (message == WM_MOUSEMOVE) {
            onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }

        return hr;
    }

    void tick() {
        if (getKeyState(VK_LEFT)) {
            rect = moveRect(rect, rect.left - 5, rect.top);
        }

        if (getKeyState(VK_RIGHT)) {
            rect = moveRect(rect, rect.left + 5, rect.top);
        }

        if (getKeyState(VK_UP)) {
            rect = moveRect(rect, rect.left, rect.top - 5);
        }

        if (getKeyState(VK_DOWN)) {
            rect = moveRect(rect, rect.left, rect.top + 5);
        }

        gfx.invalidateWindow();
    }

    HRESULT onPaint() {
        HRESULT hr = S_OK;

        gfx.beginDraw();

        gfx.clear();

        gfx.drawRect(rect, black);

        D2D1_RECT_F layoutRect = D2D1_RECT_F{0, 0, 100, 100};

        const wchar_t* text = L"Hello World test 123456 sdfsfdsdfsdfsdfsdf";
        gfx.drawText(text, layoutRect, 1);
        gfx.drawRect(layoutRect, blue);

        gfx.render();

        hr = gfx.endDraw();

        return hr;
    }

    void onLeftClick(int x, int y) {
        rect = moveRect(rect, x, y);
        queue.enqueue("trig");
    }

    void onMouseMove(int x, int y) {
    }

    void destroy() {
        gfx.destroy();
        queue.enqueue("quit");
        audioThread.join();
    }
};

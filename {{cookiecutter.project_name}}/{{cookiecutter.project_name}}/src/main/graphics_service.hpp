#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>
#include <windowsx.h>
#include <winnt.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

#include "constants.hpp"
#include "graphics_elts.hpp"
#include "util.hpp"

class GraphicsService {
public:
    GraphicsService(HWND window, HRESULT& hr) : window(window) {
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
        if (FAILED(hr)) {
            return;
        }

        hr = createGraphicsResources();
        if (FAILED(hr)) {
            return;
        }

        hr = createFontResources();
        if (FAILED(hr)) {
            return;
        }
    }

    void beginDraw() {
        BeginPaint(window, &ps);
        renderTarget->BeginDraw();
    }

    HRESULT endDraw() {
        HRESULT hr = renderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) {
            std::cout << "recreating graphics resources" << std::endl;
            releaseGraphicsResources();
            createGraphicsResources();
            hr = S_OK;
        }

        EndPaint(window, &ps);

        return hr;
    }

    void clear() {
        renderTarget->Clear(white);
    }

    void drawRect(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, int z=0) {
        GraphicsElt elt = makeRect(rect, color, z);
        drawQueue.push_back(elt);
    }

    void drawText(const wchar_t* text, const D2D1_RECT_F& rect, int z=0) {
        GraphicsElt elt = makeText(text, rect, z);
        drawQueue.push_back(elt);
    }

    void render() {
        std::sort(drawQueue.begin(), drawQueue.end(), drawQueueCompare);
        while (!drawQueue.empty()) {
            drawGraphicsElt(drawQueue.back());
            drawQueue.pop_back();
        }
    }

    void invalidateWindow() {
        InvalidateRect(window, NULL, FALSE);
    }

    void destroy() {
        releaseGraphicsResources();
        safeRelease(&factory);
        safeRelease(&writeFactory);
        safeRelease(&textFormat);
    }

private:
    HWND window;
    PAINTSTRUCT ps;
    ID2D1Factory* factory{NULL};
    ID2D1HwndRenderTarget* renderTarget{NULL};
    ID2D1SolidColorBrush* blackBrush{NULL};
    IDWriteFactory* writeFactory{NULL};
    IDWriteTextFormat* textFormat{NULL};
    std::vector<GraphicsElt> drawQueue;

    HRESULT createGraphicsResources() {
        HRESULT hr;

        RECT windowClientRect;
        GetClientRect(window, &windowClientRect);

        D2D1_SIZE_U windowClientRectSize = D2D1::SizeU(windowClientRect.right, windowClientRect.bottom);

        hr = factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(window, windowClientRectSize),
            &renderTarget
        );
        if (FAILED(hr)) {
            return hr;
        }

        hr = renderTarget->CreateSolidColorBrush(black, &blackBrush);
        if (FAILED(hr)) {
            return hr;
        }

        return hr;
    }

    HRESULT createFontResources() {
        HRESULT hr;

        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&writeFactory)
        );
        if (FAILED(hr)) {
            return hr;
        }

        hr = writeFactory->CreateTextFormat(
            L"Consolas",                      // font family name
            NULL,                             // pointer to font collection object (?)
            DWRITE_FONT_WEIGHT_NORMAL,        // font weight
            DWRITE_FONT_STYLE_NORMAL,         // font style
            DWRITE_FONT_STRETCH_NORMAL,       // font stretch
            11.0f * 96.0f/72.0f,              // logical size of font in DIPs
            L"en-US",                         // locale name
            &textFormat                       // output
        );
        if (FAILED(hr)) {
            return hr;
        }

        return hr;
    }

    HRESULT _drawRect(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color) {
        ID2D1SolidColorBrush* newBrush = NULL;

        // does this ever actually fail?
        HRESULT hr = renderTarget->CreateSolidColorBrush(color, &newBrush);
        if (FAILED(hr)) {
            return hr;
        }

        renderTarget->FillRectangle(rect, newBrush);

        safeRelease(&newBrush);

        return hr;
    }

    void _drawText(const wchar_t* text, const D2D1_RECT_F& layoutRect) {
        renderTarget->DrawText(
            text,
            wcslen(text),
            textFormat,
            layoutRect,
            blackBrush
        );
    }

    static bool drawQueueCompare(const GraphicsElt& a, const GraphicsElt& b) {
        return a.z > b.z;
    }

    void drawGraphicsElt(const GraphicsElt& elt) {
        if (elt.tag == G_RECT) {
            _drawRect(elt.rect, elt.color);
        } else if (elt.tag == G_TEXT) {
            _drawText(elt.text, elt.rect);
        }
    }

    void releaseGraphicsResources() {
        safeRelease(&renderTarget);
        safeRelease(&blackBrush);
    }
};

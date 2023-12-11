#pragma once

#include <d2d1.h>
#include <sstream>
#include <string>
#pragma comment(lib, "d2d1")

template <typename T>
inline std::wstring toHexString(const T& t) {
    std::wstringstream ss;
    ss << "0x" << std::hex << t;
    return ss.str();
}

inline void messageBox(const wchar_t* message) {
    MessageBox(NULL, message, L"", NULL);
}

inline void messageBoxError(HRESULT hr) {
    wchar_t buffer[64];
    swprintf_s(buffer, 64, L"error:\n%s", toHexString(hr).c_str());
    MessageBox(NULL, buffer, L"", NULL);
}

template <class T>
inline void safeRelease(T **resource) {
    if (*resource != NULL) {
        (*resource)->Release();
        *resource = NULL;
    }
}

inline D2D1_RECT_F moveRect(D2D1_RECT_F rect, float x, float y) {
    float w = rect.right - rect.left;
    float h = rect.bottom - rect.top;
    return D2D1::RectF(x, y, x + w, y + h);
}

inline void printRect(D2D1_RECT_F rect) {
    wchar_t buffer[128];

    swprintf_s(
        buffer,
        128,
        L"rect:\nleft: %.2f, top: %.2f, right: %.2f, bottom: %.2f",
        rect.left,
        rect.top,
        rect.right,
        rect.bottom
    );

    messageBox(buffer);
}

inline void storeInstanceData(HWND window, LPARAM lParam) {
    CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
    void* createParam = createStruct->lpCreateParams;
    SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)createParam);
}

inline void storeInstanceData(HWND window, void* p_instanceData) {
    SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)p_instanceData);
}

inline void* getInstanceData(HWND window) {
    return (void*)GetWindowLongPtrW(window, GWLP_USERDATA);
}

inline bool getKeyState(UINT keyCode) {
    return (GetKeyState(keyCode) & 0x8000);
}

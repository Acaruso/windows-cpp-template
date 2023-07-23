#pragma once

#include <d2d1.h>
#pragma comment(lib, "d2d1")

enum GraphicsEltTag { G_RECT, G_TEXT };

struct GraphicsElt {
    GraphicsEltTag tag;
    int z{0};
    D2D1_COLOR_F color;
    D2D1_RECT_F rect;
    const wchar_t* text;
};

inline GraphicsElt makeRect(const D2D1_RECT_F& rect, const D2D1_COLOR_F& color, int z=0) {
    GraphicsElt elt;
    elt.tag = G_RECT;
    elt.z = z;
    elt.rect = rect;
    elt.color = color;
    return elt;
}

inline GraphicsElt makeText(const wchar_t* text, const D2D1_RECT_F& rect, int z=0) {
    GraphicsElt elt;
    elt.tag = G_TEXT;
    elt.z = z;
    elt.text = text;
    elt.rect = rect;
    return elt;
}

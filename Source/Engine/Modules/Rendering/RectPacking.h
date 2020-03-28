#pragma once

#include <EASTL/vector.h>

namespace Packing
{
    struct Rect
    {
        int x{ 0 };
        int y{ 0 };

        int w{ 0 };
        int h{ 0 };

        bool wasPacked{ false };

        int ordering{ -1 };
    };

    void RowPackRects(eastl::vector<Rect>& rects, int width, int height);
    void SkylinePackRects(eastl::vector<Rect>& rects, int width, int height);
}
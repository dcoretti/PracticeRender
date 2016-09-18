#pragma once

#include "../Types/Math.h"

#include <cstdint>

namespace Text {
    struct Glyph {
        int codePoint;
        int advanceWidth; // not scaled by xScale
        int leftSideBearing;

        // scaled width and height with apron for tree placement ONLY (not as accurate as x1-x0 y1-y0
        int width, height;

        // unscaled character coordinates sourced from GetCodepointBox (Y increases up)
        int x0, y0, x1, y1; 
        Vec2 uv[6];    

        // Texture location
        int bitmapX;    
        int bitmapY;
    };


    class Font {
    public:
        static const char start;
        static const char end;
        static const int apron;
        Font() : 
            scaleX(1.0f),
            scaleY(1.0f),
            heightPixels(0.0f),
            ascent(0),
            descent(0),
            lineGap(0),
            glyphs(nullptr) {
        }

        int textureW;
        int textureH;
        float scaleX;
        float scaleY;

        // defined prior to rasterizing
        float heightPixels; // font height in pixels
        int ascent;
        int descent;
        int lineGap;

        Glyph *glyphs{ nullptr };
		int *kernTable{ nullptr };
    };


    int getCodePointPairKey(int a, int b);

    void loadFontToTexture(Font &font, char * fname, unsigned char * texture, int texW, int texH);

    struct GlyphNode {
        GlyphNode() = default;
        ~GlyphNode() {
            if (child[0] != nullptr) {
                delete child[0];
            }
            if (child[1] != nullptr) {
                delete child[1];
            }
        }
        void initChildren(int firstHalfW, int firstHalfH, bool isChildTopBottom);

        // x,y relative to the whole grid, not just the parent
        // 0,0 is at the bottom left
        int x{ 0 };
        int y{ 0 };
        int width{ 0 };
        int height{ 0 };

        // child represents alternating upper/lower half and left/right half of area defined by the parent
        GlyphNode *child[2]{ nullptr, nullptr };
        Glyph *glyph{ nullptr };
    };
    
    unsigned int fontTextToGeometry(Font &f, char *text, unsigned int *vao);
    
}
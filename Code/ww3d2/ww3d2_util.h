#ifndef WW3D2_UTIL_H
#define WW3D2_UTIL_H

#include <cstdint>
#include "matrix4.h"

class WW3D2_Util{
public:

    // Utilities
    static Vector4 Convert_Color(unsigned color){
        Vector4 col;
        col[3]=((color&0xff000000)>>24)/255.0f;
        col[0]=((color&0xff0000)>>16)/255.0f;
        col[1]=((color&0xff00)>>8)/255.0f;
        col[2]=((color&0xff)>>0)/255.0f;
        //	col=Vector4(1.0f,1.0f,1.0f,1.0f);
        return col;
    }

    // ----------------------------------------------------------------------------
    //
    // Convert RGBA color from float vector to 32 bit integer
    //
    // ----------------------------------------------------------------------------
    static unsigned int Convert_Color(const Vector4& color){
        return Convert_Color(reinterpret_cast<const Vector3&>(color),color[3]);
    }

    // ----------------------------------------------------------------------------
    //
    // Convert RGBA color from float vector to 32 bit integer
    // Note: Color vector needs to be clamped to [0...1] range!
    //
    // ----------------------------------------------------------------------------
    static unsigned int Convert_Color(const Vector3& color, const float alpha){
        constexpr float scale = 255.0f;
        unsigned int col;

        uint8_t r = (uint8_t)(color.X * scale);
        uint8_t g = (uint8_t)(color.Y * scale);
        uint8_t b = (uint8_t)(color.Z * scale);
        uint8_t a = (uint8_t)(alpha * scale);
        col = (a << 24) | (r << 16) | (g << 8) | (b << 0);
        return col;
    }

    // ----------------------------------------------------------------------------
    //
    // Clamp color vertor to [0...1] range
    //
    // ----------------------------------------------------------------------------
    static void Clamp_Color(Vector4& color){
        for (int i=0;i<4;++i) {
            float f=(color[i]<0.0f) ? 0.0f : color[i];
            color[i]=(f>1.0f) ? 1.0f : f;
        }
    }

    static unsigned int Convert_Color_Clamp(const Vector4& color){
        Vector4 clamped_color=color;
        WW3D2_Util::Clamp_Color(clamped_color);
        return Convert_Color(reinterpret_cast<const Vector3&>(clamped_color),clamped_color[3]);
    }

    static void	Set_Alpha (const float alpha, unsigned int &color){
        unsigned char *component = (unsigned char*) &color;

        component [3] = 255.0f * alpha;
    }

};

#endif // WW3D2_UTIL_H

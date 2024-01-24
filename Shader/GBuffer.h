//
// Created by why on 2024/1/8.
//

#ifndef CHILLSOFTWARERENDERER_GBUFFER_H
#define CHILLSOFTWARERENDERER_GBUFFER_H

#include "../Math.h"


class GBuffer {
private:
    static GBuffer *Buffer;
    int width = 0;
    int height = 0;
    std::vector<float> DepthBuffer;
    std::vector<Vec3f> NormalBuffer;

public:
    GBuffer(int width_, int height_) : width(width_), height(height_) {
        DepthBuffer.resize(width_ * height_);
        NormalBuffer.resize(width_ * height_);
        Buffer = this;
    }

    inline float GetWidth() const {
        return width;
    }

    inline float GetHeight() const {
        return height;
    }

    inline Vec2i GetSize() const {
        return Vec2i{width, height};
    }

    inline float GetDepth(int x, int y) {
        return DepthBuffer[x * width + y];
    }

    inline Vec3f GetNormal(int x, int y) {
        return NormalBuffer[x * width + y];
    }

    inline float SetDepth(int x, int y, float depth) {
        return DepthBuffer[x * width + y];
    }

    inline Vec3f SetNormal(int x, int y, Vec3f normal) {
        return NormalBuffer[x * width + y];
    }

    static GBuffer *GetGBuffer() {
        return Buffer;
    }

};


#endif //CHILLSOFTWARERENDERER_GBUFFER_H

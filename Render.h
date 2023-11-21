//
// Created by why on 2023/11/15.
//

#ifndef TINYRENDERLESSONCODE_RENDER_H
#define TINYRENDERLESSONCODE_RENDER_H
#include "Math.h"
#include "TGAImage.h"
#include "vector"
#include "Model.h"
#include "Shader/IShader.h"

static void triangle(Model* model ,Vec3f *pts, Vec2f* textures, TGAImage &image, std::vector<std::vector<float>>& ZBuffer, IShader* Shader)
{
    Vec2f bboxmin(image.get_width()-1,  image.get_height()-1);
    Vec2f bboxmax(0, 0);
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0.0f, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.0f, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    Vec3i P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            float z = 0;
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            Vec2f PTexture(0,0);
            for (int i = 0; i < 3; i++) {
                z += pts[i].z * bc_screen.raw[i];//插值得到三角形是某点的z值
//                std::cout<<pts[i].z << bc_screen.raw[i]<<std::endl;
            }
            //插值拿到纹理uv
            for (int i = 0; i < 3; i++) {
                PTexture.u += textures[i].u * bc_screen.raw[i];
                PTexture.v += textures[i].v * bc_screen.raw[i];
            }
            //在三角形外部则跳过这个点继续循环
            if(ZBuffer[P.x][P.y] < z)
            {
                ZBuffer[P.x][P.y] = z;
                TGAColor Color;
				Shader->fragment(bc_screen, Color);
                image.set(P.x, P.y, Color);
            }
        }
    }

}

static void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1-x0;
    int dy = y1-y0;
    int derror2 = std::abs(dy)*2;
    int error2 = 0;
    int y = y0;
    for (int x=x0; x<=x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1>y0?1:-1);
            error2 -= dx*2;
        }
    }
}

#endif //TINYRENDERLESSONCODE_RENDER_H

//
// Created by why on 2023/11/15.
//

#ifndef TINYRENDERLESSONCODE_RENDER_H
#define TINYRENDERLESSONCODE_RENDER_H
#include "Math.h"
#include "TGAImage.h"
#include "vector"
#include "memory"
#include "Model.h"
#include "Shader/IShader.h"

static TGAColor ACESToneMapping(Vec3f color, float adapted_lum = 1.0f)
{

    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;
    color *= adapted_lum;
    color = color / 255.0f;
    TGAColor MappingColor;
    for(int i=0;i<3;i++)
    {
        MappingColor.raw[i] = (color.raw[i] * (A * color + B).raw[i]) / (color.raw[i] * (C * color.raw[i] + D) + E) * 255;
    }

    return MappingColor;
}


static void triangle(std::shared_ptr<Model>& model ,Vec4f *pts, TGAImage &image, std::vector<std::vector<float>>& ZBuffer, std::shared_ptr<IShader>& Shader)
{
    Vec3f pts_vec3f[3] =
    {
        pts[0].ToVec3() / pts[0].w,
        pts[1].ToVec3() / pts[1].w,
        pts[2].ToVec3() / pts[2].w,
    };
    Vec2f bboxmin(image.get_width()-1,  image.get_height()-1);
    Vec2f bboxmax(0, 0);
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0.0f, std::min(bboxmin.x, pts_vec3f[i].x));
        bboxmin.y = std::max(0.0f, std::min(bboxmin.y, pts_vec3f[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts_vec3f[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts_vec3f[i].y));
    }
    Vec3i P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            float z = 0;
            Vec3f bc_screen = barycentric(pts_vec3f[0], pts_vec3f[1], pts_vec3f[2], P);
			Vec3f bc_screen_copy = bc_screen;
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            for(int i = 0; i < 3; i++)
            {
                bc_screen.raw[i] = bc_screen.raw[i] / pts[i].w;
            }
            float Z_n = 1.0f / (bc_screen.x + bc_screen.y + bc_screen.z);
            bc_screen *= Z_n;
            for (int i = 0; i < 3; i++) {
                z += pts_vec3f[i].z * bc_screen_copy.raw[i];//插值得到三角形是某点的z值
            }
            Shader->ScreenCoord = {P.x, P.y};
            //在三角形外部则跳过这个点继续循环
            if(ZBuffer[P.x][P.y] > z)
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

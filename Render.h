//
// Created by why on 2023/11/15.
//

#ifndef CHILLSOFTWARERENDERER_RENDER_H
#define CHILLSOFTWARERENDERER_RENDER_H

#include "Math.h"
#include "TGAImage.h"
#include "vector"
#include "memory"
#include "Model.h"
#include "Shader/IShader.h"

static TGAColor ACESToneMapping(Vec3f color, float adapted_lum = 1.0f) {

    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;
    color *= adapted_lum;
    color = color / 255.0f;
    TGAColor MappingColor;
    for (int i = 0; i < 3; i++) {
        MappingColor.raw[i] =
                (color.raw[i] * (A * color + B).raw[i]) / (color.raw[i] * (C * color.raw[i] + D) + E) * 255;
    }

    return MappingColor;
}


static void triangle(std::shared_ptr<Model> &model, const std::vector<Vec4f> &pts, TGAImage &image,
                     std::vector<std::vector<float>> &ZBuffer, std::shared_ptr<IShader> &Shader) {
    std::vector<Vec3f> PointsCoord
            {
                    pts[0].ToVec3() / pts[0].w,
                    pts[1].ToVec3() / pts[1].w,
                    pts[2].ToVec3() / pts[2].w,
            };
    Vec2f bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2f bboxmax(0, 0);
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        bboxmin.x = std::max(0.0f, std::min(bboxmin.x, PointsCoord[i].x));
        bboxmin.y = std::max(0.0f, std::min(bboxmin.y, PointsCoord[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, PointsCoord[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, PointsCoord[i].y));
    }
    Vec3i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f LinearInterpBaryCoord = barycentric(PointsCoord[0], PointsCoord[1], PointsCoord[2], P);
            if (LinearInterpBaryCoord.x < 0 || LinearInterpBaryCoord.y < 0 || LinearInterpBaryCoord.z < 0) continue;
            float z = ChillMathUtility::TriangleBarycentricInterp(PointsCoord, LinearInterpBaryCoord).z;
            Vec3f CorrectBaryCoord = ChillMathUtility::PerspectiveCorrectInterpolation(pts, LinearInterpBaryCoord);
            Shader->ScreenCoord = {P.x, P.y};
            //在三角形外部则跳过这个点继续循环
            if (ZBuffer[P.x][P.y] > z) {
                ZBuffer[P.x][P.y] = z;
                TGAColor Color;
                Shader->fragment(CorrectBaryCoord, Color);
                image.set(P.x, P.y, Color);
            }
        }
    }

}

static void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

namespace ChillRender{
	enum class EFaceCulling : uint8_t {
		BackFacingCulling = 0,
		FrontFacingCulling = 1,
		DisableFacingCulling = 2
	};

	static bool FaceCulling(const std::vector<Vec3f>& WorldCoords, const Camera& Camera, EFaceCulling FaceCullingType = EFaceCulling::BackFacingCulling)
	{
		Vec3f TriangleFaceNormal;
		switch (FaceCullingType)
		{
		case EFaceCulling::BackFacingCulling:
			TriangleFaceNormal = (WorldCoords[2] - WorldCoords[0]) ^ (WorldCoords[1] - WorldCoords[0]);
			return TriangleFaceNormal.normlize() * (WorldCoords[0] - Camera.Location) >= 0;
		case EFaceCulling::FrontFacingCulling:
			TriangleFaceNormal = (WorldCoords[2] - WorldCoords[0]) ^ (WorldCoords[1] - WorldCoords[0]);
			return TriangleFaceNormal.normlize() * (WorldCoords[0] - Camera.Location) < 0;
		case EFaceCulling::DisableFacingCulling:
			return true;
		}
		return true;
	}


}

#endif //CHILLSOFTWARERENDERER_RENDER_H

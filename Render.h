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
#include "Shader/GBuffer.h"

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

	static Vec3f GetNormalInCameraSpace(const std::vector<VertexOut>& Vertex, const VertexOut& Point)
	{
		Vec3f bn = Point.VertexNormal.normlize();
		Mat3x3 AI = Mat3x3{(Vertex[1].CameraSpaceCoord - Vertex[0].CameraSpaceCoord).ToStdVector(),
						   (Vertex[2].CameraSpaceCoord - Vertex[0].CameraSpaceCoord).ToStdVector(),
						   bn.ToStdVector()}.Inverse();
		Matrix<3, 1, float> T = (AI * Matrix<3, 1, float>{
			{Vertex[1].UV.u - Vertex[0].UV.u},
			{Vertex[2].UV.u - Vertex[0].UV.u},
			{0} }).Normalize();
		Matrix<3, 1, float> B = (AI * Matrix<3, 1, float>{
			{Vertex[1].UV.v - Vertex[0].UV.v},
			{Vertex[2].UV.v - Vertex[0].UV.v},
			{0}}).Normalize();

		Mat3x3 TBN{
			{T.raw[0][0], T.raw[1][0], T.raw[2][0]},
			{B.raw[0][0], B.raw[1][0], B.raw[2][0]},
			{bn.x,        bn.y,        bn.z},
		};
		Vec3f tangentSpaceNormal = Point.TangentSpaceNormal;
		Matrix Mat_n = TBN.Transpose() * tangentSpaceNormal.ToMatrix();//TBN^T is same as TBN^-1
		Vec3f n = Mat_n.ToVec3f().normlize();
		return n;
	}


	static void DrawTriangle(const std::vector<VertexOut> &Vertex, const std::shared_ptr<Model>& Model, TGAImage &image,
		Buffer<float> &ZBuffer, std::shared_ptr<IShader> &Shader) {
		std::vector<Vec2f>ScreenPointsCoords
			{
				Vertex[0].ScreenSpaceCoord,
				Vertex[1].ScreenSpaceCoord,
				Vertex[2].ScreenSpaceCoord
			};
		std::vector<Vec3f>NDCCoords
			{
				Vertex[0].NDC,
				Vertex[1].NDC,
				Vertex[2].NDC
			};
		std::vector<Vec4f>ClipSpaceCoords
			{
				Vertex[0].ClipSpaceCoord,
				Vertex[1].ClipSpaceCoord,
				Vertex[2].ClipSpaceCoord
			};

		using BoxType = ChillMathUtility::AABBHelper::BoxType<float, 2>;
		BoxType Triangle_AABB = ChillMathUtility::GetAABB<float, 2>(ScreenPointsCoords);
		BoxType Screen_AABB{Vec2f(0.f, 0.f), Vec2f(image.get_width() - 1, image.get_height() - 1)};
		bool bAABBIntersect;
		BoxType ResultAABB = ChillMathUtility::GetAABB_Intersect<float, 2>(Triangle_AABB, Screen_AABB, bAABBIntersect);
		if(!bAABBIntersect) return;

		const Vec2i AABBMin(static_cast<int>(ResultAABB.first.x), static_cast<int>(ResultAABB.first.y));
		const Vec2i AABBMax(static_cast<int>(ResultAABB.second.x), static_cast<int>(ResultAABB.second.y));
		Vec2i P;
		for (P.x = AABBMin.x; P.x <= AABBMax.x; P.x++) {
			for (P.y = AABBMin.y; P.y <= AABBMax.y; P.y++) {
				Vec3f LinearInterpBaryCoord = barycentric(ScreenPointsCoords[0], ScreenPointsCoords[1], ScreenPointsCoords[2], P);
				if (LinearInterpBaryCoord.x < 0 || LinearInterpBaryCoord.y < 0 || LinearInterpBaryCoord.z < 0) continue;
				float z = ChillMathUtility::TriangleBarycentricInterp(NDCCoords, LinearInterpBaryCoord).z;
				Vec3f CorrectBaryCoord = ChillMathUtility::PerspectiveCorrectInterpolation(ClipSpaceCoords, LinearInterpBaryCoord);
				VertexOut Point = ChillMathUtility::TriangleBarycentricInterp(Vertex, CorrectBaryCoord);
				Point.TangentSpaceNormal = Model->getNormal(Point.UV);
				Vec3f Normal = ChillRender::GetNormalInCameraSpace(Vertex, Point);
				Point.VertexNormal = Normal;
				Shader->ScreenCoord = {P.x, P.y};
				//在三角形外部则跳过这个点继续循环
				if (ZBuffer[P.x][P.y] > z) {
					ZBuffer[P.x][P.y] = z;
					TGAColor Color;
					Shader->fragment(Point, Color);
					image.set(P.x, P.y, Color);
				}
			}
		}
	}


	static void Render(std::shared_ptr<Model>& Model, std::shared_ptr<IShader>& Shader, Camera& Camera,TGAImage& Image, Buffer<float>& ZBuffer, EFaceCulling FaceCulling)
	{
		for (int iFace = 0; iFace < Model->nfaces(); iFace++) {
			const std::vector<int> &face = Model->getface(iFace);
			std::vector<VertexOut>Vertex(3);
			std::vector<Vec3f>WorldCoords(3);
			for (int nVertex = 0; nVertex < 3; nVertex++) {
				VertexOut& V  = Vertex[nVertex];
				Vertex[nVertex].ClipSpaceCoord = Shader->vertex(iFace, nVertex, V).ToVec4f();
				Vertex[nVertex].WorldSpaceCoord = Model->getvert(iFace, nVertex);
				Vertex[nVertex].UV = Model->getuv(iFace, nVertex);
				Vertex[nVertex].NDC = ChillMathUtility::PerspectiveDivide(Vertex[nVertex].ClipSpaceCoord);
				Vertex[nVertex].ScreenSpaceCoord = Camera.NDC2ScreenSpaceCoord(Vertex[nVertex].NDC);

				WorldCoords[nVertex] = Vertex[nVertex].WorldSpaceCoord;
			}

			if (ChillRender::FaceCulling(WorldCoords, Camera, FaceCulling)) //back face culling
			{
				ChillRender::DrawTriangle(Vertex, Model, Image, ZBuffer, Shader);
			}

		}
	}

}


#endif //CHILLSOFTWARERENDERER_RENDER_H

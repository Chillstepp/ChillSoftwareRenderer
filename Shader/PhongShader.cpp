//
// Created by why on 2024/1/5.
//

#include "PhongShader.h"


PhongShader::~PhongShader() {

}

Matrix<4, 1, float> PhongShader::vertex(int iface, int nthvert, VertexOut& Vertex) {

	Matrix<4, 1, float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
	gl_vertex = camera.ViewMatrix * gl_vertex;
    Vertex.CameraSpaceCoord = Mat4x1::Proj(gl_vertex, true);
    gl_vertex = camera.ProjectionMatrix * gl_vertex;
	Vertex.VertexNormal = Matrix<4, 1, float>::Proj(
        Uniform_MIT * Mat4x1::Embed(model->getNormal(iface, nthvert))
    );
    return gl_vertex;
}

bool PhongShader::fragment(VertexOut Point, TGAColor &color) {
    //uv interp
    Vec2f uv = Point.UV;
    Vec3f n = Point.VertexNormal;
    Vec3f l = (Uniform_M.RemoveHomogeneousDim() * scene->LightDir.ToMatrix()).ToVec3f().normlize();
    Vec3f r = ChillMathUtility::ReflectedVec(l, n); // reflected light
    Vec3f Center2Eye = (Uniform_M.RemoveHomogeneousDim() * (camera.Location - camera.LookTo).normlize().ToMatrix()).ToVec3f();
    float spec = std::pow(std::max(r * Center2Eye, 0.0f), 20 + model->getSpecular(uv));
    float diff = std::max(0.f, -n * l);

    NormalBuffer[ScreenCoord.x][ScreenCoord.y] = Mat4x1::Proj(camera.ViewMatrix.Inverse() * Mat4x1::Embed(n));

    //Shadow
    Vec3f p = Point.CameraSpaceCoord;
    Matrix<4, 1, float> CorrespondingPointInShadowBuffer = Uniform_MShadow * Matrix<4, 1, float>::Embed(p);
    CorrespondingPointInShadowBuffer /= CorrespondingPointInShadowBuffer.raw[3][0];
    Vec3f CorrespondingPoint{CorrespondingPointInShadowBuffer.raw[0][0], CorrespondingPointInShadowBuffer.raw[1][0],
                             CorrespondingPointInShadowBuffer.raw[2][0]};
    float shadowFactor = 1.0f;
    constexpr float shadowK = 1.0f;
    float shadowBias = shadowK * (1.0f - n * l);
    //PCSS
    auto findAverageBlockerDis = [&shadowBias](const Vec2i &point, const float receiverDis,
                                               const Buffer<float>& depthBufferInLightView,
                                               const int sampleRadius = 1) {
        bool overEdge = point.x - sampleRadius < 0 or point.x + sampleRadius >= depthBufferInLightView.Width
                        or point.y - sampleRadius < 0 or point.y + sampleRadius >= depthBufferInLightView.Height;
        if (overEdge) return 0.0f;
        int totalSampleTimes = 0;
        float totalBlockerDis = 0;
        for (int dx = -sampleRadius; dx <= sampleRadius; dx++) {
            for (int dy = -sampleRadius; dy <= sampleRadius; dy++) {

                float ClosetDis = depthBufferInLightView[point.x + dx][point.y + dy] * 255.0f/2.0f + 255.0f/2.0f;
                if (ClosetDis < receiverDis - shadowBias) {
                    totalBlockerDis += ClosetDis;
                    ++totalSampleTimes;
                }
            }
        }
        return totalBlockerDis / static_cast<float>(totalSampleTimes);
    };
    if ((int) CorrespondingPoint.x < DepthBuffer.Width &&
        (int) CorrespondingPoint.y < DepthBuffer.Height) {
        bool bBlock = false;
		float DepthAfterViewport = DepthBuffer[(int) CorrespondingPoint.x][(int) CorrespondingPoint.y] * 255.0f/2.0f + 255.0f/2.0f;
        if (DepthAfterViewport < CorrespondingPoint.z - shadowBias) {
            bBlock = true;
        }
        ShadowBuffer[ScreenCoord.x][ScreenCoord.y] = bBlock ? 1 : 0;
        float averageBlockerDis = findAverageBlockerDis({(int) CorrespondingPoint.x, (int) CorrespondingPoint.y},
                                                       CorrespondingPoint.z, DepthBuffer, 40);
        float RecevierDisatance = CorrespondingPoint.z;
        constexpr float lightRadius = 50.0f;
        float Penumbra = (RecevierDisatance - averageBlockerDis) * lightRadius / averageBlockerDis;
        PenumbraBuffer[ScreenCoord.x][ScreenCoord.y] = Penumbra;
    }
    TGAColor c = model->diffuse(uv);
    color = c;

    for (int i = 0; i < 3; i++)
        color.raw[i] = std::min<float>((float) c.raw[i] * (0.3f + 1.0f * diff + 1.6f * spec), 255.0f);

    return false;

}
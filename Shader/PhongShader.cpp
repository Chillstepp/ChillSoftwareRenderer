//
// Created by why on 2024/1/5.
//

#include "PhongShader.h"


PhongShader::~PhongShader() {

}

Matrix<4, 1, float> PhongShader::vertex(int iface, int nthvert) {
    Varying_uv[nthvert] = model->getuv(iface, nthvert);

    Matrix<4, 1, float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
    gl_vertex = camera.ViewMatrix * gl_vertex;
    Varying_tri[nthvert] = Mat4x1::Proj(gl_vertex, true);
    gl_vertex = camera.ProjectionMatrix * gl_vertex;

    Varying_normal[nthvert] = Matrix<4, 1, float>::Proj(
        Uniform_MIT * Mat4x1::Embed(model->getNormal(iface, nthvert))
    );
    return gl_vertex;
}

bool PhongShader::fragment(Vec3f bar, TGAColor &color) {
    //uv interp
    Vec2f uv = ChillMathUtility::TriangleBarycentricInterp(Varying_uv, bar);

    //tangent-space-normal-mapping
    Vec3f bn = ChillMathUtility::TriangleBarycentricInterp(Varying_normal, bar).normlize();

    Mat3x3 AI = Mat3x3{(Varying_tri[1] - Varying_tri[0]).ToStdVector(), (Varying_tri[2] - Varying_tri[0]).ToStdVector(),
                       bn.ToStdVector()}.Inverse();
    Matrix<3, 1, float> T = (AI * Matrix<3, 1, float>{
            {Varying_uv[1].u - Varying_uv[0].u},
            {Varying_uv[2].u - Varying_uv[0].u},
            {0}}).Normalize();

    Matrix<3, 1, float> B = (AI * Matrix<3, 1, float>{
            {Varying_uv[1].v - Varying_uv[0].v},
            {Varying_uv[2].v - Varying_uv[0].v},
            {0}}).Normalize();
    Mat3x3 TBN{
            {T.raw[0][0], T.raw[1][0], T.raw[2][0]},
            {B.raw[0][0], B.raw[1][0], B.raw[2][0]},
            {bn.x,        bn.y,        bn.z},
    };
    Vec3f tangentSpaceNormal = model->getNormal(uv);
    Matrix Mat_n = TBN.Transpose() * tangentSpaceNormal.ToMatrix();//TBN^T is same as TBN^-1
    Vec3f n = Mat_n.ToVec3f().normlize();
    Vec3f l = (Uniform_M.RemoveHomogeneousDim() * scene->LightDir.ToMatrix()).ToVec3f().normlize();
    Vec3f r = ChillMathUtility::ReflectedVec(l, n); // reflected light
    Vec3f Center2Eye = (Uniform_M.RemoveHomogeneousDim() * (camera.Location - camera.LookTo).normlize().ToMatrix()).ToVec3f();
    float spec = std::pow(std::max(r * Center2Eye, 0.0f), 20 + model->getSpecular(uv));
    float diff = std::max(0.f, -n * l);

    NormalBuffer[ScreenCoord.x][ScreenCoord.y] = Mat4x1::Proj(camera.ViewMatrix.Inverse() * Mat4x1::Embed(n));

    //Shadow
    Vec3f p = ChillMathUtility::TriangleBarycentricInterp(Varying_tri, bar);
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
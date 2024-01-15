//
// Created by why on 2024/1/5.
//

#ifndef TINYRENDERLESSONCODE_PHONGSHADER_H
#define TINYRENDERLESSONCODE_PHONGSHADER_H

#include "IShader.h"

class PhongShader: public IShader
{
private:
    std::shared_ptr<Model> model = nullptr;//@todo: use weak ptr plz.
    const std::vector<std::vector<float>>&DepthBuffer;
    std::vector<std::vector<float>>&ShadowBuffer;
    std::vector<std::vector<float>>&PenumbraBuffer;
    std::vector<std::vector<Vec3f>>&NormalBuffer;
    std::vector<std::vector<Mat3x3>>&TBNBuffer;
    std::vector<std::vector<float>>&ScreenPosWBuffer;
    Vec3f LightDir;
    Mat4x4 ProjectionMat;
    Mat4x4 ModelViewMat;
    Mat4x4 ViewPortMat;
    Vec3f Eye;
    Vec3f Center;


    std::vector<Vec2f>Varying_uv{3, {0, 0}};
    std::vector<Vec3f>Varying_tri{3, {0, 0, 0}};
    std::vector<Vec3f>Varying_normal{3, {0, 0, 0}};
    std::vector<float>Varying_w{0.0f, 0.0f, 0.0f};
    Mat4x4 Uniform_M;
    Mat4x4 Uniform_MIT;
    Mat4x4 Uniform_MShadow;//transform framebuffer screen coordinates to shadowbuffer screen coordinates

public:
    explicit PhongShader(std::shared_ptr<Model>& model_, Mat4x4 ProjectionMat_, Mat4x4 ModelViewMat_,Mat4x4 ViewPortMat_, Vec3f LightDir_,
                         Vec3f Eye_, Vec3f Center_, Mat4x4 Uniform_MShadow_, const std::vector<std::vector<float>>&DepthBuffer_, std::vector<std::vector<float>>&ShadowBuffer_,
                         std::vector<std::vector<float>>&PenumbraBuffer_, std::vector<std::vector<Vec3f>>&NormalBuffer_, std::vector<std::vector<Mat3x3>>&TBNBuffer_,
                         std::vector<std::vector<float>>&ScreenPosWBuffer_) :
            model(model_), ProjectionMat(ProjectionMat_),ModelViewMat(ModelViewMat_), ViewPortMat(ViewPortMat_), LightDir(LightDir_),
            Uniform_MShadow(Uniform_MShadow_), DepthBuffer(DepthBuffer_),Eye(Eye_), Center(Center_), ShadowBuffer(ShadowBuffer_),
            PenumbraBuffer(PenumbraBuffer_), NormalBuffer(NormalBuffer_), TBNBuffer(TBNBuffer_), ScreenPosWBuffer(ScreenPosWBuffer_)
    {
        Uniform_M = ModelViewMat_;
        Uniform_MIT = Uniform_M.Inverse().Transpose();
        // framebuffer screen coordinates -> world coordinates -> shadowbuffer screen coordinates

    }
    ~PhongShader() override;
    virtual Matrix<4, 1, float> vertex(int iface, int nthvert) override;
    virtual bool fragment(Vec3f bar, TGAColor &color) override;
};

#endif //TINYRENDERLESSONCODE_PHONGSHADER_H

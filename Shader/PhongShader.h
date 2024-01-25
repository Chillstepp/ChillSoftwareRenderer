//
// Created by why on 2024/1/5.
//

#ifndef CHILLSOFTWARERENDERER_PHONGSHADER_H
#define CHILLSOFTWARERENDERER_PHONGSHADER_H

#include "IShader.h"
#include "../Scene.h"

class PhongShader : public IShader {
private:
    std::shared_ptr<Model> model = nullptr;//@todo: use weak ptr plz.
    std::shared_ptr<Scene> scene;
    Camera camera;

    const std::vector<std::vector<float>> &DepthBuffer;
    std::vector<std::vector<float>> &ShadowBuffer;// Depth in light-view space
    std::vector<std::vector<float>> &PenumbraBuffer;//PCSS Penumbra Buffer
    std::vector<std::vector<Vec3f>> &NormalBuffer;
    std::vector<std::vector<Mat3x3>> &TBNBuffer;



    std::vector<Vec2f> Varying_uv{3, {0, 0}};
    std::vector<Vec3f> Varying_tri{3, {0, 0, 0}};
    std::vector<Vec3f> Varying_WorldPos{3, {0, 0, 0}};
    std::vector<Vec3f> Varying_normal{3, {0, 0, 0}};
    std::vector<float> Varying_w{0.0f, 0.0f, 0.0f};
    Mat4x4 Uniform_M;
    Mat4x4 Uniform_MIT;
    Mat4x4 Uniform_MShadow;//transform framebuffer screen coordinates to shadowbuffer screen coordinates

public:
    explicit PhongShader(std::shared_ptr<Model> &model_,
                         const Camera& camera_,
                         const std::shared_ptr<Scene>& scene_,
                         const std::vector<std::vector<float>> &DepthBuffer_,
                         std::vector<std::vector<float>> &ShadowBuffer_,
                         std::vector<std::vector<float>> &PenumbraBuffer_,
                         std::vector<std::vector<Vec3f>> &NormalBuffer_,
                         std::vector<std::vector<Mat3x3>> &TBNBuffer_) :
            model(model_), camera(camera_), scene(scene_), DepthBuffer(DepthBuffer_),ShadowBuffer(ShadowBuffer_),
            PenumbraBuffer(PenumbraBuffer_), NormalBuffer(NormalBuffer_), TBNBuffer(TBNBuffer_){
        Uniform_M = camera.ViewMatrix;
        Uniform_MIT = Uniform_M.Inverse().Transpose();
		Uniform_MShadow = (camera.ViewportMatrix * camera.ProjectionMatrix * lookat(scene->LightPos, camera.LookTo, camera.Up)) * camera.ViewMatrix.Inverse();
    }

    ~PhongShader() override;

    virtual Matrix<4, 1, float> vertex(int iface, int nthvert) override;

    virtual bool fragment(Vec3f bar, TGAColor &color) override;
};

#endif //CHILLSOFTWARERENDERER_PHONGSHADER_H

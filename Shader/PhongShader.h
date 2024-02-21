//
// Created by why on 2024/1/5.
//

#ifndef CHILLSOFTWARERENDERER_PHONGSHADER_H
#define CHILLSOFTWARERENDERER_PHONGSHADER_H

#include "IShader.h"
#include "../Scene.h"
#include "GBuffer.h"

class PhongShader : public IShader {
private:
    std::shared_ptr<Model> model = nullptr;//@todo: use weak ptr plz.
    std::shared_ptr<Scene> scene;
    const Camera& camera;

    const Buffer<float>& DepthBuffer;
    Buffer<float>& ShadowBuffer;// Record which pixels are occluded
    Buffer<float>& PenumbraBuffer;//PCSS Method Penumbra
    Buffer<Vec3f>& NormalBuffer;// Vertex normal

    Mat4x4 Uniform_M;
    Mat4x4 Uniform_MIT;
    Mat4x4 Uniform_MShadow;//transform framebuffer screen coordinates to shadowbuffer screen coordinates

public:
    explicit PhongShader(std::shared_ptr<Model> &model_,
                         const Camera& camera_,
                         const std::shared_ptr<Scene>& scene_) :
            model(model_), camera(camera_), scene(scene_),
            DepthBuffer(*GBuffer::Get().GetBuffer<float>("DepthBuffer")),
            ShadowBuffer(*GBuffer::Get().GetBuffer<float>("ShadowBuffer")),
            PenumbraBuffer(*GBuffer::Get().GetBuffer<float>("PenumbraBuffer")),
            NormalBuffer(*GBuffer::Get().GetBuffer<Vec3f>("NormalBuffer")){
        Uniform_M = camera.ViewMatrix;
        Uniform_MIT = Uniform_M.Inverse().Transpose();
		Uniform_MShadow = (camera.ViewportMatrix * camera.ProjectionMatrix * lookat(scene->LightPos, camera.LookTo, camera.Up)) * camera.ViewMatrix.Inverse();
    }

    ~PhongShader() override;

    virtual Matrix<4, 1, float> vertex(int iface, int nthvert, VertexOut& Vertex) override;

    virtual bool fragment(VertexOut Vertex, TGAColor &color) override;
};

#endif //CHILLSOFTWARERENDERER_PHONGSHADER_H

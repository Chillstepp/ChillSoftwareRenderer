//
// Created by Chillstep on 2023/11/19.
//

#ifndef TINYRENDERLESSONCODE_SHADER_ISHADER_H_
#define TINYRENDERLESSONCODE_SHADER_ISHADER_H_

#include <memory>
#include "../Math.h"
#include "../TGAImage.h"
#include "../Model.h"

class IShader
{
 public:
	virtual ~IShader() = default;
	virtual Matrix<4,1,float> vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

class FlatShader: public IShader
{
 private:
	Matrix<3, 3, float> Varying_tri;
	Model* model = nullptr;
	Mat4x4 ProjectionMat;
	Mat4x4 ModelViewMat;
	Mat4x4 ViewPortMat;
	Vec3f LightDir{0,0,-1};

 public:
	explicit FlatShader(Model* model_, Mat4x4 ProjectionMat_, Mat4x4 ModelViewMat_,
		Mat4x4 ViewPortMat_, Vec3f LightDir_): model(model_), ProjectionMat(ProjectionMat_), ModelViewMat(ModelViewMat_), ViewPortMat(ViewPortMat_), LightDir(LightDir_){}
	~FlatShader() override;
	virtual Matrix<4, 1, float> vertex(int iface, int nthvert) override;
	virtual bool fragment(Vec3f bar, TGAColor &color) override;
};


class GouraudShader: public IShader
{
private:
	Vec3f Varying_intensity;
	std::shared_ptr<Model> model = nullptr;
	Mat4x4 ProjectionMat;
	Mat4x4 ModelViewMat;
	Mat4x4 ViewPortMat;
	Vec3f LightDir{0,0,-1};

public:
	explicit GouraudShader(std::shared_ptr<Model>& model_, Mat4x4 ProjectionMat_, Mat4x4 ModelViewMat_,
						Mat4x4 ViewPortMat_, Vec3f LightDir_): model(model_), ProjectionMat(ProjectionMat_), ModelViewMat(ModelViewMat_), ViewPortMat(ViewPortMat_), LightDir(LightDir_){}
	~GouraudShader() override;
	virtual Matrix<4, 1, float> vertex(int iface, int nthvert) override;
	virtual bool fragment(Vec3f bar, TGAColor &color) override;
};

class PhongShader: public IShader
{
private:
	std::shared_ptr<Model> model = nullptr;//@todo: use weak ptr plz.
    const std::vector<std::vector<float>>&DepthBuffer;
    Vec3f LightDir;
	Mat4x4 ProjectionMat;
	Mat4x4 ModelViewMat;
	Mat4x4 ViewPortMat;
    Vec3f Eye;
    Vec3f Center;


    std::vector<Vec2f>Varying_uv;
    std::vector<Vec3f>Varying_tri;
    Mat4x4 Uniform_M;
    Mat4x4 Uniform_MIT;
    Mat4x4 Uniform_MShadow;//transform framebuffer screen coordinates to shadowbuffer screen coordinates

public:
	explicit PhongShader(std::shared_ptr<Model>& model_, Mat4x4 ProjectionMat_, Mat4x4 ModelViewMat_,Mat4x4 ViewPortMat_, Vec3f LightDir_,
                         Vec3f Eye_, Vec3f Center_, Mat4x4 Uniform_MShadow_, const std::vector<std::vector<float>>&DepthBuffer_) :
                model(model_), ProjectionMat(ProjectionMat_),ModelViewMat(ModelViewMat_), ViewPortMat(ViewPortMat_), LightDir(LightDir_),
                Uniform_MShadow(Uniform_MShadow_), DepthBuffer(DepthBuffer_),Eye(Eye_), Center(Center_)
    {
        Varying_uv.reserve(3);
        Varying_tri.reserve(3);
        Uniform_M = ModelViewMat_;
        Uniform_MIT = Uniform_M.Inverse().Transpose();
        // framebuffer screen coordinates -> world coordinates -> shadowbuffer screen coordinates

    }
	~PhongShader() override;
	virtual Matrix<4, 1, float> vertex(int iface, int nthvert) override;
	virtual bool fragment(Vec3f bar, TGAColor &color) override;
};


class DepthShder : public IShader
{
public:
    std::shared_ptr<Model> model = nullptr;//@todo: use weak ptr plz.
    std::vector<Vec3f>varying_tri;
    Mat4x4 ProjectionMat;
    Mat4x4 ModelViewMat;
    Mat4x4 ViewPortMat;

    DepthShder(std::shared_ptr<Model>& model_, Mat4x4 ProjectionMat_, Mat4x4 ModelViewMat_,Mat4x4 ViewPortMat_):
            model(model_), ProjectionMat(ProjectionMat_),ModelViewMat(ModelViewMat_), ViewPortMat(ViewPortMat_)
    {
        varying_tri.reserve(3);
    }
    virtual Matrix<4, 1, float> vertex(int iface, int nthvert) override;
    virtual bool fragment(Vec3f bar, TGAColor &color) override;
};


class SSAOShader : public IShader
{
public:
    std::shared_ptr<Model> model = nullptr;//@todo: use weak ptr plz.
    std::vector<Vec3f>varying_tri;
    Mat4x4 ProjectionMat;
    Mat4x4 ModelViewMat;
    Mat4x4 ViewPortMat;

    SSAOShader(std::shared_ptr<Model>& model_, Mat4x4 ProjectionMat_, Mat4x4 ModelViewMat_,Mat4x4 ViewPortMat_):
    model(model_), ProjectionMat(ProjectionMat_),ModelViewMat(ModelViewMat_), ViewPortMat(ViewPortMat_)
    {
        varying_tri.reserve(3);
    }
    virtual Matrix<4, 1, float> vertex(int iface, int nthvert) override;
    virtual bool fragment(Vec3f bar, TGAColor &color) override;

};

#endif //TINYRENDERLESSONCODE_SHADER_ISHADER_H_

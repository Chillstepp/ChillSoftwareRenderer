//
// Created by Chillstep on 2024/2/6.
//

#ifndef CHILLSOFTWARERENDERER_SHADER_SKYBOXSHADER_H_
#define CHILLSOFTWARERENDERER_SHADER_SKYBOXSHADER_H_

#include "IShader.h"
#include "../Scene.h"

class SkyBoxShader : public IShader
{
 private:
	std::shared_ptr<Model> model = nullptr;
	std::shared_ptr<Scene> scene;
	const Camera& camera;

	std::vector<Vec3f> Varying_tri{3, {0, 0, 0}};

	Mat4x4 Uniform_M;
	Mat4x4 Uniform_MIT;
 public:
	explicit SkyBoxShader(std::shared_ptr<Model> &model_, const Camera& camera_, const std::shared_ptr<Scene>& scene_) :
	model(model_), camera(camera_), scene(scene_){
		Uniform_M = camera.ViewMatrix;
		Uniform_MIT = Uniform_M.Inverse().Transpose();
	}

	~SkyBoxShader() override;

	virtual Matrix<4, 1, float> vertex(int iface, int nthvert, VertexOut& Point) override;

	virtual bool fragment(VertexOut Point, TGAColor &color) override;
};

#endif //CHILLSOFTWARERENDERER_SHADER_SKYBOXSHADER_H_

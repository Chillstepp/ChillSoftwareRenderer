//
// Created by Chillstep on 2024/2/6.
//

#include "SkyBoxShader.h"
SkyBoxShader::~SkyBoxShader()
{

}
Matrix<4, 1, float> SkyBoxShader::vertex(int iface, int nthvert, VertexOut& Vertex)
{
	Matrix<4, 1, float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
	Varying_tri[nthvert] = Mat4x1::Proj(gl_vertex, true);
	gl_vertex = camera.ViewMatrix * gl_vertex;
    Vertex.CameraSpaceCoord = Mat4x1::Proj(gl_vertex, true);
	gl_vertex = camera.ProjectionMatrix * gl_vertex;
    Vertex.VertexNormal = Matrix<4, 1, float>::Proj(
            Uniform_MIT * Mat4x1::Embed(model->getNormal(iface, nthvert))
    );
	return gl_vertex;
}
bool SkyBoxShader::fragment(VertexOut Point, TGAColor& color)
{
	Vec2f uv = Point.UV;
	EFaceOrientation CurrentFaceOri = EFaceOrientation::Unknown;
	if(std::all_of(Varying_tri.begin(), Varying_tri.end(), [](const Vec3f& In){return In.x > 0;}))
	{
		CurrentFaceOri = EFaceOrientation::Right;
        std::swap(uv.u, uv.v);
        uv.v = 1 - uv.v;
       // uv.u = 1 - uv.u;
	}
	else if(std::all_of(Varying_tri.begin(), Varying_tri.end(), [](const Vec3f& In){return In.y > 0;}))
	{
		CurrentFaceOri = EFaceOrientation::Top;
	}
	else if(std::all_of(Varying_tri.begin(), Varying_tri.end(), [](const Vec3f& In){return In.z > 0;}))
	{
		CurrentFaceOri = EFaceOrientation::Front;
	}
	else if(std::all_of(Varying_tri.begin(), Varying_tri.end(), [](const Vec3f& In){return In.x < 0;}))
	{
		CurrentFaceOri = EFaceOrientation::Left;
        std::swap(uv.u, uv.v);
        uv.v = 1 - uv.v;
        uv.u = 1 - uv.u;
	}
	else if(std::all_of(Varying_tri.begin(), Varying_tri.end(), [](const Vec3f& In){return In.y < 0;}))
	{
		CurrentFaceOri = EFaceOrientation::Bottom;
        uv.v = 1 - uv.v;
	}
	else if(std::all_of(Varying_tri.begin(), Varying_tri.end(), [](const Vec3f& In){return In.z < 0;}))
	{
		CurrentFaceOri = EFaceOrientation::Back;
	}
	TGAColor c = model->diffuseSkyBox(uv, CurrentFaceOri);
	color = c;

	return false;
}

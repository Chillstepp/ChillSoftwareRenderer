//
// Created by Chillstep on 2023/11/19.
//

#include "IShader.h"


Matrix<4, 1, float> FlatShader::vertex(int iface, int nthvert)
{
	std::vector<int> Face = model->getface(iface);
	Matrix<4,1,float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(Face.at(nthvert * 2)));
	gl_vertex = ProjectionMat * ModelViewMat * gl_vertex;
	for(int i=0; i<3; i++)
	{
		Varying_tri.raw[i][nthvert] = gl_vertex.raw[i][0]/gl_vertex.raw[3][0];
	}
	gl_vertex = ViewPortMat * gl_vertex;
	return gl_vertex;
}
bool FlatShader::fragment(Vec3f bar, TGAColor& color)
{
	std::vector<Vec3f> TriangleVert = {
		{Varying_tri.raw[0][0], Varying_tri.raw[1][0], Varying_tri.raw[2][0]},
		{Varying_tri.raw[0][1], Varying_tri.raw[1][1], Varying_tri.raw[2][1]},
		{Varying_tri.raw[0][2], Varying_tri.raw[1][2], Varying_tri.raw[2][2]}
	};
	Vec3f norm =  (TriangleVert[2] - TriangleVert[0])^(TriangleVert[1] - TriangleVert[0]);

	float intensity = std::clamp((norm.normlize() * LightDir), 0.0f, 1.0f);
	color = TGAColor(255*intensity,255*intensity,255*intensity,255);
	return true;
}
FlatShader::~FlatShader()
{

}

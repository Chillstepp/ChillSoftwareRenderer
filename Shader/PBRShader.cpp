//
// Created by Chillstep on 2024/2/26.
//

#include "PBRShader.h"
PBRShader::~PBRShader()
{

}
Matrix<4, 1, float> PBRShader::vertex(int iface, int nthvert, VertexOut& Vertex)
{
	Matrix<4, 1, float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
	gl_vertex = camera.ViewMatrix * gl_vertex;
	Vertex.CameraSpaceCoord = Mat4x1::Proj(gl_vertex, true);
	gl_vertex = camera.ProjectionMatrix * gl_vertex;
	Vertex.VertexNormal = Matrix<4, 1, float>::Proj(
		Uniform_MIT * Mat4x1::Embed(model->getNormal(iface, nthvert))
	);
	return gl_vertex;
}
bool PBRShader::fragment(VertexOut Point, TGAColor& color)
{
	//uv interp
	Vec2f uv = Point.UV;
	Vec3f n = Point.VertexNormal;

	return false;
}

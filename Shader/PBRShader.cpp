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

Vec3f PBRShader::fresnelSchlick(float cosTheta, const Vec3f& F0)
{
	return F0 + (Vec3f::One - F0) * std::pow(std::clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

bool PBRShader::fragment(VertexOut Point, TGAColor& color)
{
	//uv interp
	Vec2f uv = Point.UV;
	Vec3f n = Point.VertexNormal;
	float roughness = model->getRoughness(uv).x;
	float metallic = model->getMetallic(uv).x;
	Vec3f albedo = model->getAlbedo(uv);

	Vec3f V = (camera.Location - Point.WorldSpaceCoord).normlize();
	Vec3f L = (scene->LightPos - Point.WorldSpaceCoord).normlize();
	Vec3f H = (V + L).normlize();//half vector
	float distance = (scene->LightPos - Point.WorldSpaceCoord).norm();
	float attenuation = 1.0f / (distance * distance);
	Vec3f lightColors(23.47, 21.31, 20.79);
	Vec3f radiance = lightColors * attenuation;

	Vec3f F0(0.04, 0.04, 0.04);//平面基础反射率
	F0 = ChillMathUtility::Vec3fLerp(F0, albedo, metallic);//根据金属度作插值,对于非金属表面F0始终为0.04。对于金属表面，我们根据初始的F0和表现金属属性的反射率进行线性插值。
	Vec3f F = fresnelSchlick(std::max(H*V, 0.0f), F0);



	return false;
}
float PBRShader::DistributionGGX(Vec3f N, Vec3f H, float roughness)
{
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = std::max(N * H, 0.0f);
	float NdotH2 = NdotH*NdotH;

	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = M_PI * denom * denom;

	return num / denom;
}
float PBRShader::GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r*r) / 8.0f;

	float num   = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return num / denom;
}
float PBRShader::GeometrySmith(Vec3f N, Vec3f V, Vec3f L, float roughness)
{
	float NdotV = std::max(N * V, 0.0f);
	float NdotL = std::max(N * L, 0.0f);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

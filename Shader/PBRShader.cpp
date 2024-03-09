//
// Created by Chillstep on 2024/2/26.
//

#include "PBRShader.h"
#include "../Render.h"

PBRShader::~PBRShader()
{

}
Matrix<4, 1, float> PBRShader::vertex(int iface, int nthvert, VertexOut& Vertex)
{
	Matrix<4, 1, float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
	gl_vertex = camera.ViewMatrix * gl_vertex;
	Vertex.CameraSpaceCoord = Mat4x1::Proj(gl_vertex, true);
	gl_vertex = camera.ProjectionMatrix * gl_vertex;
	//view space normal
    //Normal's transformation should not use homogeneous element!
	Vertex.VertexNormal = Matrix<4, 1, float>::Proj(Uniform_MIT * Mat4x1::Embed(model->getNormal(iface, nthvert)), false).normlize();

	return gl_vertex;
}


bool PBRShader::fragment(VertexOut Point, TGAColor& color)
{
    return fragment_IBL(Point, color);
}






Vec3f PBRShader::fresnelSchlick(float cosTheta, const Vec3f& F0)
{
    return F0 + (Vec3f::One - F0) * std::pow(std::clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float PBRShader::DistributionGGX(Vec3f N, Vec3f H, float roughness)
{
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = std::max(N * H, 0.0f);
	float NdotH2 = NdotH*NdotH;

	float num    = a2;
	float denom  = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = M_PI * denom * denom;

	return num / denom;
}

float PBRShader::GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r*r) / 8.0f;

	float nom   = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return nom / denom;
}

float PBRShader::GeometrySmith(Vec3f N, Vec3f V, Vec3f L, float roughness)
{
	float NdotV = std::max(N * V, 0.0f);
	float NdotL = std::max(N * L, 0.0f);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

bool PBRShader::fragment_PointLight(const VertexOut &Point, TGAColor &color) {
    //uv interp
    Vec2f uv = Point.UV;
    //Normal's transformation should not use homogeneous element!
    Vec3f N =  Mat4x1::Proj(Uniform_MT * Mat4x1::Embed(Point.VertexNormal), false).normlize();
    float roughness = model->getRoughness(uv).z;
    float metallic = model->getMetallic(uv).z;
    Vec3f emission = model->getEmission(uv);
    Vec3f albedo = model->getAlbedo(uv);
    float ao = model->getAO(uv).z;

    Vec3f V = (camera.Location - Point.WorldSpaceCoord).normlize();
    Vec3f Lo(0.0f,0.0f,0.0f);

    for(Vec3f LightPos: {Vec3f(-5,-5, 5), Vec3f(-5, 5,10), Vec3f(5,-5,10), Vec3f(5,5,10)}) {


        Vec3f L = (LightPos - Point.WorldSpaceCoord).normlize();
        Vec3f H = (V + L).normlize();//half vector
        Vec3f radiance(3,3,3);


        Vec3f F0(0.04f, 0.04f, 0.04f);//平面基础反射率
        F0 = ChillMathUtility::Vec3fLerp(F0, albedo, metallic);//根据金属度作插值,对于非金属表面F0始终为0.04。对于金属表面，我们根据初始的F0和表现金属属性的反射率进行线性插值。
        Vec3f F = fresnelSchlick(std::max(H * V, 0.0f), F0);

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);

        //BRDF Ready:
        Vec3f nominator = NDF * G * F;
        float denominator = 4.0f * std::max(N * V, 0.0f) * std::max(N * L, 0.0f) + 0.0001;
        Vec3f specular = nominator / denominator;

        Vec3f kS = F;//反射的比值
        Vec3f kD = Vec3f(1.0f, 1.0f, 1.0f) - kS;//折射的比值
        kD *= 1.0f - metallic;//金属不会折射光线，因此不会有漫反射。所以如果表面是金属的，我们会把系数kD变为0

        float NdotL = std::max(N * L, 0.0f);

        for (int i = 0; i < 3; i++) {
            Lo.raw[i] += (albedo.raw[i] * kD.raw[i] / (float) M_PI + specular.raw[i]) * radiance.raw[i] * NdotL;
        }
    }
    //加一个环境光照
    Vec3f ambient;
    for(int i = 0; i < 3; i++)
    {
        ambient.raw[i] = Vec3f(0.05f, 0.05f, 0.05f).raw[i] * albedo.raw[i] * ao;
    }

    Vec3f colorNormalize = ambient + Lo + emission;
    //colorNormalize = N;
    for(int i = 0; i < 3; i++)
    {
        colorNormalize.raw[i] = colorNormalize.raw[i] / (colorNormalize.raw[i] + 1.0f);
        colorNormalize.raw[i] = std::pow(colorNormalize.raw[i], 1.0f/2.2f);
    }

    color = TGAColor(std::min(colorNormalize.raw[0] * 255.0f, 255.0f), std::min(colorNormalize.raw[1] * 255.0f, 255.0f), std::min(colorNormalize.raw[2] * 255.0f, 255.0f), 255);
    return true;
}

//diffuse irradiance Precomputation
bool PBRShader::fragment_IBL(const VertexOut &Point, TGAColor &color) {

    //uv interp
    Vec2f uv = Point.UV;
    //Normal's transformation should not use homogeneous element!
    Vec3f N =  Mat4x1::Proj(Uniform_MT * Mat4x1::Embed(Point.VertexNormal), false).normlize();

    float roughness = model->getRoughness(uv).z;
    float metallic = model->getMetallic(uv).z;
    Vec3f emission = model->getEmission(uv);
    Vec3f albedo = model->getAlbedo(uv);
    float ao = model->getAO(uv).z;

    Vec3f V = (camera.Location - Point.WorldSpaceCoord).normlize();
    Vec3f Lo(0.0f,0.0f,0.0f);
    Vec3f F0_basic(0.04f, 0.04f, 0.04f);//平面基础反射率

    //use IBL as ambient lighting
    Vec3f F0 = ChillMathUtility::Vec3fLerp(F0_basic, albedo, metallic);
    Vec3f kS = fresnelSchlick(std::max((N * V), 0.0f), F0);
    Vec3f kD = Vec3f(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;

    Vec2f IrrMapUV;
    EFaceOrientation FaceOrientation = ChillSampling::Calc_CubemapUV_From_Normal(N, IrrMapUV);
    Vec3f irradiance = Vec3f(scene->SkyBox->diffuseSkyBox(IrrMapUV ,FaceOrientation)) / 255.0f;
    Vec3f ambient(0, 0, 0);
    for(int i = 0; i < 3; i++)
    {
        ambient.raw[i] = kD.raw[i] * irradiance.raw[i] * irradiance.raw[i] * albedo.raw[i] * ao;
    }

	Vec3f r = ChillMathUtility::ReflectedVec(-V, N);
	Vec2f lut_uv = Vec2f(N * V, roughness);
	Vec3f lut_sample = model->getLUT(lut_uv);
	float specular_scale = lut_sample.x;
	float specular_bias = lut_sample.y;
	Vec3f specular = F0 * specular_scale + specular_bias;
	Vec2f Prefilter_uv;
	auto FaceOrientation2 = ChillSampling::Calc_CubemapUV_From_Normal(r, Prefilter_uv);
	Vec3f prefilter_color = Vec3f(scene->SkyBox->diffuseSkyBox(Prefilter_uv ,FaceOrientation2)) / 255.0f;
	for(int i = 0; i < 3; i++)
	{
		specular.raw[i] = specular.raw[i] * prefilter_color.raw[i] * prefilter_color.raw[i];
	}
    Vec3f colorNormalize = ambient + specular + Lo + emission;



    for(int i = 0; i < 3; i++)
    {
        colorNormalize.raw[i] = colorNormalize.raw[i] / (colorNormalize.raw[i] + 1.0f);
        colorNormalize.raw[i] = std::pow(colorNormalize.raw[i], 1.0f/2.2f);
    }

    color = TGAColor(std::min(colorNormalize.raw[0] * 255.0f, 255.0f), std::min(colorNormalize.raw[1] * 255.0f, 255.0f), std::min(colorNormalize.raw[2] * 255.0f, 255.0f), 255);
    return true;
}

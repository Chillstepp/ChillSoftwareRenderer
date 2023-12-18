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
    float d = gl_vertex.raw[3][0];
    gl_vertex.raw[0][0] = gl_vertex.raw[0][0] / gl_vertex.raw[3][0];
    gl_vertex.raw[1][0] = gl_vertex.raw[1][0] / gl_vertex.raw[3][0];
    gl_vertex.raw[2][0] = gl_vertex.raw[2][0] / gl_vertex.raw[3][0];
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



GouraudShader::~GouraudShader() {

}

Matrix<4, 1, float> GouraudShader::vertex(int iface, int nthvert) {
    Matrix<4,1,float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
    Vec3f norm = model->getNormal(iface, nthvert);
    Varying_intensity.raw[nthvert] = std::clamp(norm*LightDir, 0.0f, 1.0f);

    gl_vertex = ViewPortMat * ProjectionMat * ModelViewMat * gl_vertex;
    gl_vertex /= gl_vertex.raw[3][0];
    return gl_vertex;
}

bool GouraudShader::fragment(Vec3f bar, TGAColor &color) {
    float intensity = Varying_intensity * bar;
    color = TGAColor(255*intensity,255*intensity,255*intensity,255);
    return true;
}


PhongShader::~PhongShader() {

}

Matrix<4, 1, float> PhongShader::vertex(int iface, int nthvert) {
    const std::vector<int>& Face = model->getface(iface);
    Vec2f uv = model->getuv(Face[nthvert * 3 + 1]);
    Varying_uv[nthvert] = uv;

    Matrix<4,1,float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(Face[nthvert * 3]));
    gl_vertex = ViewPortMat * ProjectionMat * ModelViewMat * gl_vertex;
    gl_vertex /= gl_vertex.raw[3][0];

    Varying_tri[nthvert] = {gl_vertex.raw[0][0], gl_vertex.raw[1][0], gl_vertex.raw[2][0]};
    Varying_normal[nthvert] = model->getNormal(iface, nthvert);
    std::cout<< iface<< " "<< nthvert<<" " << Varying_normal[nthvert]<<std::endl;

    return gl_vertex;
}

bool PhongShader::fragment(Vec3f bar, TGAColor &color) {
    //uv插值
    Vec2f uv;
    for(int i=0;i<3;i++)
    {
        uv.u += Varying_uv[i].u*bar.raw[i];
        uv.v += Varying_uv[i].v*bar.raw[i];
    }
    //坐标插值
    Vec3f p(0,0,0);
    for(int i=0;i<3;i++)
    {
        p = p + Varying_tri[i] * bar.raw[i];
    }
    Matrix<4,1,float>CorrespondingPointInShadowBuffer = Uniform_MShadow * Matrix<4,1,float>::Embed(p);
    CorrespondingPointInShadowBuffer /= CorrespondingPointInShadowBuffer.raw[3][0];
    Vec3f CorrespondingPoint{CorrespondingPointInShadowBuffer.raw[0][0], CorrespondingPointInShadowBuffer.raw[1][0], CorrespondingPointInShadowBuffer.raw[2][0]};
	float shadowFactor = 1.0f;
//	if((int)CorrespondingPoint.x < DepthBuffer.size() && (int)CorrespondingPoint.y < DepthBuffer[(int)CorrespondingPoint.x].size())
//	{shadowFactor = 0.3f + 0.7f*(DepthBuffer[(int)CorrespondingPoint.x][(int)CorrespondingPoint.y] < CorrespondingPoint.z);
//	}
    //tangent-space-normal-mapping
    Vec3f bn = {0.0f, 0.0f, 0.0f};
    std::for_each(Varying_normal.begin(), Varying_normal.end(),
                  [&bn, &bar = std::as_const(bar)](const Vec3f& in)
                  {bn = bn + in * bar[];}
                  );
    Mat3x3 AI = Mat3x3{ (Varying_tri[1] - Varying_tri[0]).ToStdVector(), (Varying_tri[2] - Varying_tri[0]).ToStdVector(), bn.ToStdVector()}.Inverse();
    Mat3x3 TBN = AI * Mat3x3{{Varying_uv[1].u - Varying_uv[0].u , Varying_uv[1].v - Varying_uv[0].v, 0},
                             {Varying_uv[2].u - Varying_uv[0].u , Varying_uv[2].v - Varying_uv[0].v, 0},
                             {0, 0, 0}};
    Vec3f tagentSpaceNormal = model->getNormal(uv);
    auto Mat_n = TBN.Transpose() * Matrix<3, 1, float>{{tagentSpaceNormal.x}, {tagentSpaceNormal.y}, {tagentSpaceNormal.z}};//TBN^T is same as TBN^-1
    Vec3f n = Vec3f{Mat_n[0][0], Mat_n[1][0], Mat_n[2][0]}.normlize();
    Vec3f l = Matrix<4,1,float>::Proj(Uniform_M*Mat4x4::Embed(LightDir)).normlize();
    Vec3f r = (l - n*(n*l*2.f)).normlize(); // reflected light
    Vec3f Center2Eye = {0,0,-1};
    float spec = std::pow(std::max(r * Center2Eye, 0.0f), 20 + model->getSpecular(uv)); // we're looking from z-axis
    float diff = std::max(0.f, -n*l);
    auto x = model->getNormal(uv);
   // std::cout<<"r:"<<r<<"n:"<<n<<"l:"<<l<<"uv:"<<uv<<"normal:"<< x<<"diff:"<<diff<<" spec:"<<spec<<std::endl;
    TGAColor c = model->diffuse(uv);
    color = c;

	for (int i = 0; i < 3; i++) color.raw[i] = std::min<float>(c.raw[i]*(0.3 + 1.0*diff + 1.6*spec)*shadowFactor, 255);

    return false;

}

Matrix<4, 1, float> DepthShder::vertex(int iface, int nthvert) {
    const std::vector<int>& Face = model->getface(iface);
    Matrix<4,1,float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(Face[nthvert * 3]));

    gl_vertex = ViewPortMat*ProjectionMat*ModelViewMat*gl_vertex;
    gl_vertex /= gl_vertex.raw[3][0];
    varying_tri[nthvert] = {gl_vertex.raw[0][0], gl_vertex.raw[1][0], gl_vertex.raw[2][0]};
    return gl_vertex;
}

bool DepthShder::fragment(Vec3f bar, TGAColor &color) {
    Vec3f p(0,0,0);
    for(int i=0;i<3;i++)
    {
        p = p + varying_tri[i] * bar.raw[i];
    }
    float factor = p.z;
    color = TGAColor((int)255*factor, (int)255*factor, (int)255*factor, 255);
    return true;
}

Matrix<4, 1, float> SSAOShader::vertex(int iface, int nthvert) {
    const std::vector<int>& Face = model->getface(iface);
    Matrix<4,1,float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(Face[nthvert * 3]));
    gl_vertex = ViewPortMat*ProjectionMat*ModelViewMat*gl_vertex;
    gl_vertex /= gl_vertex.raw[3][0];
    varying_tri[nthvert] = {gl_vertex.raw[0][0], gl_vertex.raw[1][0], gl_vertex.raw[2][0]};
    return gl_vertex;
}

bool SSAOShader::fragment(Vec3f bar, TGAColor &color) {
    color = TGAColor(255,255,255,255);

    return true;
}

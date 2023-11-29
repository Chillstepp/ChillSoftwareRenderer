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
    const std::vector<int>& Face = model->getface(iface);
    Matrix<4,1,float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(Face[nthvert * 3]));
    Vec3f norm = model->getNormal(iface, nthvert);
    Varying_intensity.raw[nthvert] = std::clamp(norm*LightDir, 0.0f, 1.0f);

    gl_vertex = ViewPortMat * ProjectionMat * ModelViewMat * gl_vertex;
    gl_vertex.raw[0][0] = gl_vertex.raw[0][0] / gl_vertex.raw[3][0];
    gl_vertex.raw[1][0] = gl_vertex.raw[1][0] / gl_vertex.raw[3][0];
    gl_vertex.raw[2][0] = gl_vertex.raw[2][0] / gl_vertex.raw[3][0];
    return gl_vertex;
}

bool GouraudShader::fragment(Vec3f bar, TGAColor &color) {
    float intensity = Varying_intensity * bar;
//    std::cout<<intensity<<" "<< std::endl<<Varying_intensity.raw[0]<<" "<<Varying_intensity.raw[1]<<" "<<Varying_intensity.raw[2]<<std::endl<<
//    bar.raw[0]<<" "<<bar.raw[1]<<" "<<bar.raw[2]<<std::endl;
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
    return gl_vertex;
}

bool PhongShader::fragment(Vec3f bar, TGAColor &color) {
//    std::cout<<intensity<<" "<< std::endl<<Varying_intensity.raw[0]<<" "<<Varying_intensity.raw[1]<<" "<<Varying_intensity.raw[2]<<std::endl<<
//    bar.raw[0]<<" "<<bar.raw[1]<<" "<<bar.raw[2]<<std::endl;
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
    float shadowFactor = 0.3f + 0.7f*(DepthBuffer[(int)CorrespondingPoint.x][(int)CorrespondingPoint.y] < CorrespondingPoint.z);

    Vec3f n = Matrix<4,1,float>::Proj(Uniform_MIT*Mat4x4::Embed(model->getNormal(uv))).normlize();
    Vec3f l = Matrix<4,1,float>::Proj(Uniform_M*Mat4x4::Embed(LightDir)).normlize();
    Vec3f r = (n*(n*l*2.f) - l).normlize(); // reflected light
    float spec = pow(std::max(r.z, 0.0f), 20 + model->getSpecular(uv)); // we're looking from z-axis
    float diff = std::max(0.f, n*l);

    TGAColor c = model->diffuse(uv);
    color = c;



    for (int i = 0; i < 3; i++) color.raw[i] = std::min<float>(5+c.raw[i]*(1.0*diff+1.7*spec)*shadowFactor, 255);

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
    float factor = (p.z + 1)/2;
   // std::cout<<factor<<std::endl;
    color = TGAColor((int)255*factor, (int)255*factor, (int)255*factor, 255);
    return true;
}

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

    Matrix<4,1,float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
    gl_vertex = ModelViewMat * gl_vertex;
    Varying_tri[nthvert] = {gl_vertex.raw[0][0]/gl_vertex.raw[3][0], gl_vertex.raw[1][0]/gl_vertex.raw[3][0], gl_vertex.raw[2][0]/gl_vertex.raw[3][0]};
    gl_vertex = ViewPortMat * ProjectionMat * gl_vertex;
    //std::cout<<gl_vertex.raw[0][0]<<" "<<gl_vertex.raw[1][0]<<" "<<gl_vertex.raw[2][0]<<"\n";
    gl_vertex /= gl_vertex.raw[3][0];
    //std::cout<<"test:"<< gl_vertex.raw[0][0]<<" "<<gl_vertex.raw[1][0]<<" "<<gl_vertex.raw[2][0]<<"\n";


    Varying_normal[nthvert] = Matrix<4,1,float>::Proj((ModelViewMat).Inverse().Transpose() * Matrix<4,1,float>::Embed(model->getNormal(iface, nthvert)));
    //std::cout<< iface<< " "<< nthvert<<" " << Varying_normal[nthvert]<<std::endl;

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

    //tangent-space-normal-mapping
    Vec3f bn = {0.0f, 0.0f, 0.0f};
    for(int i = 0; i < 3; i++)
    {
        bn = bn + Varying_normal[i] * bar.raw[i];
    }
    bn = bn.normlize();
    Mat3x3 AI = Mat3x3{ (Varying_tri[1] - Varying_tri[0]).ToStdVector(), (Varying_tri[2] - Varying_tri[0]).ToStdVector(), bn.ToStdVector()}.Inverse();
    Matrix<3,1,float> T = (AI * Matrix<3,1,float>{
        {Varying_uv[1].u - Varying_uv[0].u} ,
        {Varying_uv[2].u - Varying_uv[0].u},
        {0}}).Normalize();

    Matrix<3,1,float> B = (AI * Matrix<3,1,float>{
            {Varying_uv[1].v - Varying_uv[0].v} ,
            {Varying_uv[2].v - Varying_uv[0].v},
            {0}}).Normalize();
    Mat3x3 TBN{
        {T.raw[0][0], T.raw[1][0], T.raw[2][0]},
        {B.raw[0][0], B.raw[1][0], B.raw[2][0]},
        {bn.x, bn.y, bn.z},
    };
    Vec3f tangentSpaceNormal = model->getNormal(uv);
    Matrix Mat_n = TBN.Transpose() * tangentSpaceNormal.ToMatrix();//TBN^T is same as TBN^-1
    Vec3f n = Mat_n.ToVec3f().normlize();
    Vec3f l = (Uniform_M.RemoveHomogeneousDim() * LightDir.ToMatrix()).ToVec3f().normlize() ;
    Vec3f r = (l - n*(n*l*2.f)).normlize(); // reflected light
    Vec3f Center2Eye = (Uniform_M.RemoveHomogeneousDim()*(Eye - Center).normlize().ToMatrix()).ToVec3f();
    float spec = std::pow(std::max(r * Center2Eye, 0.0f), 20 + model->getSpecular(uv));
    float diff = std::max(0.f, -n*l);


	//Shadow
	Vec3f p(0,0,0);
	for(int i=0;i<3;i++)//坐标插值
	{
		p = p + Varying_tri[i] * bar.raw[i];
	}
	Matrix<4,1,float>CorrespondingPointInShadowBuffer = Uniform_MShadow * Matrix<4,1,float>::Embed(p);
	CorrespondingPointInShadowBuffer /= CorrespondingPointInShadowBuffer.raw[3][0];
	Vec3f CorrespondingPoint{CorrespondingPointInShadowBuffer.raw[0][0], CorrespondingPointInShadowBuffer.raw[1][0], CorrespondingPointInShadowBuffer.raw[2][0]};
	float shadowFactor = 1.0f;
	constexpr float shadowK = 1.0f;
	float shadowBias = shadowK*(1.0f - n*l);
    //PCSS
    auto findAverageBlockerDis = [&shadowBias](const Vec2i& point, const float receiverDis, const std::vector<std::vector<float>>& depthBufferInLightView, const int sampleRadius = 1){
        bool overEdge = point.x - sampleRadius < 0 or point.x + sampleRadius >= depthBufferInLightView.size()
                or point.y - sampleRadius < 0 or point.y + sampleRadius >= depthBufferInLightView[0].size();
        if(overEdge) return 0.0f;
        int totalSampleTimes = 0;
        float totalBlockerDis = 0;
        for(int dx = -sampleRadius; dx <= sampleRadius; dx++)
        {
            for(int dy = -sampleRadius; dy <= sampleRadius; dy++)
            {

                float ClosetDis = depthBufferInLightView[point.x + dx][point.y + dy];
                if(ClosetDis < receiverDis - shadowBias)
                {
                    totalBlockerDis += ClosetDis;
                    ++ totalSampleTimes;
                }
            }
        }
        return totalBlockerDis / static_cast<float>(totalSampleTimes);
    };
	if((int)CorrespondingPoint.x < DepthBuffer.size() && (int)CorrespondingPoint.y < DepthBuffer[(int)CorrespondingPoint.x].size())
	{
        bool bBlock = false;
        if(DepthBuffer[(int)CorrespondingPoint.x][(int)CorrespondingPoint.y] < CorrespondingPoint.z - shadowBias)
        {
            bBlock = true;
        }
        ShadowBuffer[ScreenCoord.x][ScreenCoord.y] = bBlock;

        float averageBlockerDis = findAverageBlockerDis({(int)CorrespondingPoint.x, (int)CorrespondingPoint.y}, CorrespondingPoint.z, DepthBuffer, 5);
        float RecevierDisatance = CorrespondingPoint.z;
        constexpr float lightRadius = 20.0f;
        float Penumbra = (RecevierDisatance - averageBlockerDis) * lightRadius / averageBlockerDis;
        PenumbraBuffer[ScreenCoord.x][ScreenCoord.y] = Penumbra;

		//shadowFactor = 0.3f + 0.7f*(DepthBuffer[(int)CorrespondingPoint.x][(int)CorrespondingPoint.y] > CorrespondingPoint.z - shadowBias);
	}
    //ShadowBuffer[ScreenCoord.x][ScreenCoord.y] = shadowFactor
    TGAColor c = model->diffuse(uv);
    color = c;

	for (int i = 0; i < 3; i++) color.raw[i] = std::min<float>((float)c.raw[i]*(0.3f + 1.0f*diff + 1.6f*spec), 255.0f);

    return false;

}

Matrix<4, 1, float> DepthShder::vertex(int iface, int nthvert) {
    const std::vector<int>& Face = model->getface(iface);
    Matrix<4,1,float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
    //std::cout<<"0x :"<<gl_vertex.raw[0][0]<<" "<<gl_vertex.raw[1][0]<<" "<<gl_vertex.raw[2][0]<<"\n";
    gl_vertex = ModelViewMat*gl_vertex;
   // std::cout<<gl_vertex.raw[0][0]<<" "<<gl_vertex.raw[1][0]<<" "<<gl_vertex.raw[2][0]<<"\n";
    gl_vertex = ProjectionMat*gl_vertex;
   // std::cout<<gl_vertex.raw[0][0]<<" "<<gl_vertex.raw[1][0]<<" "<<gl_vertex.raw[2][0]<<"\n";
    gl_vertex = ViewPortMat*gl_vertex;
    //std::cout<<gl_vertex.raw[0][0]<<" "<<gl_vertex.raw[1][0]<<" "<<gl_vertex.raw[2][0]<<"\n";
    gl_vertex /= gl_vertex.raw[3][0];
    //std::cout<<gl_vertex.raw[0][0]<<" "<<gl_vertex.raw[1][0]<<" "<<gl_vertex.raw[2][0]<<"\n";
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
    //std::cout<<p.z<<std::endl;
    color = TGAColor((int)(factor), (int)(factor), (int)(factor), 255);
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

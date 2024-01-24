//
// Created by Chillstep on 2023/11/19.
//

#include "IShader.h"


Matrix<4, 1, float> FlatShader::vertex(int iface, int nthvert) {
    std::vector<int> Face = model->getface(iface);
    Matrix<4, 1, float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(Face.at(nthvert * 2)));
    gl_vertex = ProjectionMat * ModelViewMat * gl_vertex;
    for (int i = 0; i < 3; i++) {
        Varying_tri.raw[i][nthvert] = gl_vertex.raw[i][0] / gl_vertex.raw[3][0];
    }
    gl_vertex = ViewPortMat * gl_vertex;
    float d = gl_vertex.raw[3][0];
    gl_vertex.raw[0][0] = gl_vertex.raw[0][0] / gl_vertex.raw[3][0];
    gl_vertex.raw[1][0] = gl_vertex.raw[1][0] / gl_vertex.raw[3][0];
    gl_vertex.raw[2][0] = gl_vertex.raw[2][0] / gl_vertex.raw[3][0];
    return gl_vertex;
}

bool FlatShader::fragment(Vec3f bar, TGAColor &color) {
    std::vector<Vec3f> TriangleVert = {
            {Varying_tri.raw[0][0], Varying_tri.raw[1][0], Varying_tri.raw[2][0]},
            {Varying_tri.raw[0][1], Varying_tri.raw[1][1], Varying_tri.raw[2][1]},
            {Varying_tri.raw[0][2], Varying_tri.raw[1][2], Varying_tri.raw[2][2]}
    };
    Vec3f norm = (TriangleVert[2] - TriangleVert[0]) ^ (TriangleVert[1] - TriangleVert[0]);

    float intensity = std::clamp((norm.normlize() * LightDir), 0.0f, 1.0f);
    color = TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
    return true;
}

FlatShader::~FlatShader() {

}


GouraudShader::~GouraudShader() {

}

Matrix<4, 1, float> GouraudShader::vertex(int iface, int nthvert) {
    Matrix<4, 1, float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
    Vec3f norm = model->getNormal(iface, nthvert);
    Varying_intensity.raw[nthvert] = std::clamp(norm * LightDir, 0.0f, 1.0f);

    gl_vertex = ViewPortMat * ProjectionMat * ModelViewMat * gl_vertex;
    gl_vertex /= gl_vertex.raw[3][0];
    return gl_vertex;
}

bool GouraudShader::fragment(Vec3f bar, TGAColor &color) {
    float intensity = Varying_intensity * bar;
    color = TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
    return true;
}


Matrix<4, 1, float> DepthShder::vertex(int iface, int nthvert) {
    const std::vector<int> &Face = model->getface(iface);
    Matrix<4, 1, float> gl_vertex = Matrix<4, 1, float>::Embed(model->getvert(iface, nthvert));
    gl_vertex = ViewPortMat * ProjectionMat * ModelViewMat * gl_vertex;
    varying_tri[nthvert] = Mat4x1::Proj(gl_vertex, true);

    return gl_vertex;
}

bool DepthShder::fragment(Vec3f bar, TGAColor &color) {
    Vec3f p(0, 0, 0);
    for (int i = 0; i < 3; i++) {

        p = p + varying_tri[i] * bar.raw[i];
    }
    float factor = p.z;
    color = TGAColor((int) (factor), (int) (factor), (int) (factor), 255);

    return true;
}



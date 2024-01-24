//
// Created by why on 2023/11/13.
//

#ifndef TINYRENDERLESSONCODE_MODEL_H
#define TINYRENDERLESSONCODE_MODEL_H

#include "vector"
#include "Math.h"
#include "TGAImage.h"

class Model {
private:
    std::vector<Vec3f> Verts;
    std::vector<std::vector<int>> Faces;
    std::vector<Vec3f> Norms;
    std::vector<Vec2f> uvs;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;

    void load_texture(std::string filename, const char *suffix, TGAImage &img);

public:
    Model(const char *filename);

    ~Model();

    int nverts();

    int nfaces();

    Vec3f getvert(int i);

    Vec3f getvert(int iface, int nthvert);

    std::vector<int> getface(int idx);

    Vec2f getuv(int idx);

    Vec2f getuv(int iface, int nthvert);

    Vec3f getNormal(int iface, int nthvert);

    //法线贴图中拿法线
    Vec3f getNormal(Vec2f uvf);

    //高光
    float getSpecular(Vec2f uvf);

    //高光贴图第二种
    Vec3f getSpecularV2(Vec2f uvf);

    TGAColor diffuse(Vec2f uv);
};


#endif //TINYRENDERLESSONCODE_MODEL_H

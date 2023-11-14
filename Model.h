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
    std::vector<Vec3f>Verts;
    std::vector<std::vector<int>>Faces;
    std::vector<Vec2f>uvs;
    TGAImage diffusemap_;
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(const char* filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f getvert(int i);
    std::vector<int> getface(int idx);
    Vec2f getuv(int idx);
    TGAColor diffuse(Vec2f uv);
};


#endif //TINYRENDERLESSONCODE_MODEL_H

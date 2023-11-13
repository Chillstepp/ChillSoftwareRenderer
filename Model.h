//
// Created by why on 2023/11/13.
//

#ifndef TINYRENDERLESSONCODE_MODEL_H
#define TINYRENDERLESSONCODE_MODEL_H

#include "vector"
#include "Math.h"

class Model {
private:
    std::vector<Vec3f>Verts;
    std::vector<std::vector<int>>Faces;
public:
    Model(const char* filename);
    ~Model();
    int nverts();
    int nfaces();
    Vec3f getvert(int i);
    std::vector<int> getface(int idx);
};


#endif //TINYRENDERLESSONCODE_MODEL_H

//
// Created by Chillstep on 2023/12/10.
//

#ifndef CHILLSOFTWARERENDERER__OBJECT_H_
#define CHILLSOFTWARERENDERER__OBJECT_H_

#include "Math.h"
#include "memory"
#include "Model.h"



class Object {
public:
    Object(const std::shared_ptr<Model> &model_) : model(model_) {}

    std::shared_ptr<Model> model;
    Vec3f Location{0.0f, 0.0f, 0.0f};// X, Y, Z
    Vec3f Rotation{0.0f, 0.0f, 0.0f};// Roll, Pitch, Yaw
    Vec3f Scale{1.0f, 1.0f, 1.0f};//

};

#endif //CHILLSOFTWARERENDERER__OBJECT_H_

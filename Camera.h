//
// Created by why on 2023/12/26.
//
#include "Math.h"

#ifndef CHILLSOFTWARERENDERER_CAMERA_H
#define CHILLSOFTWARERENDERER_CAMERA_H


class Camera {
public:
    Vec3f Location{0, 0, 1.5};//Camera Location
    Vec3f LookTo{0, 0, 0};//Camera look at Location
    Vec3f Up{0, 1, 0}; //Camera's Up Vector

    Mat4x4 ViewMatrix;
    Mat4x4 ProjectionMatrix;

public:
    Camera() = delete;
    Camera(Vec3f Location_, Vec3f LookTo_, Vec3f Up_): Location(Location_), LookTo(LookTo_), Up(Up_){
        Update();
    }

    /* Update matrix related */
    void Update();

    /* Update Camera Param and Update matrix related */
    void SetCameraParam(Vec3f Location_, Vec3f LookTo_, Vec3f Up_);
};


#endif //CHILLSOFTWARERENDERER_CAMERA_H

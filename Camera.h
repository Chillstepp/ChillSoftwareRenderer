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
    Vec2i ViewportSize;
    float Near;
    float Far;

    Mat4x4 ViewMatrix;
    Mat4x4 ProjectionMatrix;
    Mat4x4 ViewportMatrix;

public:
    Camera() = delete;
    Camera(Vec3f Location_, Vec3f LookTo_, Vec3f Up_, Vec2i ViewportSize_, float Near_, float Far_):
        Location(Location_), LookTo(LookTo_), Up(Up_), ViewportSize(ViewportSize_), Near(Near_), Far(Far_)
    {
        Update();
    }

    /* Update Camera Param and Update matrix related */
    void SetCameraCoreParam(Vec3f Location_, Vec3f LookTo_, Vec3f Up_);

private:
    /* Update matrix related */
    void Update();
};


#endif //CHILLSOFTWARERENDERER_CAMERA_H

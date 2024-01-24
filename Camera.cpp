//
// Created by why on 2023/12/26.
//

#include "Camera.h"

void Camera::SetCameraParam(Vec3f Location_, Vec3f LookTo_, Vec3f Up_) {
    Location = Location_;
    LookTo   = LookTo_;
    Up       = Up_;

    Update();
}

void Camera::Update() {
    ViewMatrix = lookat(Location, LookTo, Up);
    ProjectionMatrix = projection();
}

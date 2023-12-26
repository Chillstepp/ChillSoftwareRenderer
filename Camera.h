//
// Created by why on 2023/12/26.
//
#include "Math.h"
#ifndef TINYRENDERLESSONCODE_CAMERA_H
#define TINYRENDERLESSONCODE_CAMERA_H


class Camera {
public:
    Vec3f Eye{0,0,1.5};//Camera Location
    Vec3f Center{0,0,0};//Camera look at Location
    Vec3f Up{0,1,0}; //Camera's Up Vector

    

};


#endif //TINYRENDERLESSONCODE_CAMERA_H

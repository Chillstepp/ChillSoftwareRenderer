//
// Created by why on 2023/11/13.
//

#ifndef TINYRENDERLESSONCODE_MATH_H
#define TINYRENDERLESSONCODE_MATH_H
#include <cmath>
#include "iostream"


template<typename T> struct Vec2
{
    union {
        struct {T u, v;};
        struct {T x, y;};
        T raw[2];
    };
    Vec2(): u(0), v(0) {}
    Vec2(T _u, T _v): u(_u), v(_v){}

    inline Vec2<T> operator +(const Vec2<T>& vec) const {
        return Vec2<T>{u+vec.u, v+vec.v};
    }


    inline Vec2<T> operator -(const Vec2<T>& vec) const {
        return Vec2<T>{u-vec.u, v-vec.v};
    }


    inline Vec2<T> operator *(const float& f) const {
        return Vec2<T>{u*f, v*f};
    }

    template<class> friend std::ostream& operator<<(std::ostream& s, Vec2<T>&v);

};

template<class T>
std::ostream &operator<<(std::ostream &s, Vec2<T> &v) {
    s << "(" <<v.x<<","<<v.y<<") \n";
    return s;
}


template<typename T> struct Vec3
{
    union {
        struct {T x, y, z;};
        struct {T ivert, iuv, inorm;};
        T raw[2];
    };
    Vec3(): x(0), y(0), z(0) {}
    Vec3(T _x, T _y, T _z): x(_x), y(_y), z(_z){}

    inline Vec3<T> operator ^(const Vec3<T>& vec){
        return Vec3<T>{y*vec.z - z*vec.y, x*vec.z - z*vec.x, x*vec.y - y*vec.x};
    }

    inline Vec3<T> operator +(const Vec3<T>& vec) const {
        return Vec3<T>{x+vec.x, y+vec.y, z+vec.z};
    }

    inline Vec3<T> operator -(const Vec3<T>& vec) const {
        return Vec3<T>{x-vec.x, y-vec.y, z-vec.z};
    }

    inline Vec3<T> operator *(const float& f) const {
        return Vec3<T>{x*f, y*f, z*f};
    }

    inline T operator *(const Vec3<T>& vec) const {
        return x*vec.x + y*vec.y + z*vec.z;
    }

    float norm() const{
        return std::sqrt(x*x+y*y+z*z);
    }

    Vec3<T>& normlize(T l = 1){
        *this = (*this)*(l/norm());
        return *this;
    }
    template<class> friend std::ostream& operator<<(std::ostream& s, Vec3<T>&v);

};

template<class T>
std::ostream &operator<<(std::ostream &s, Vec3<T> &v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}

using Vec3f = Vec3<float>;
using Vec2f = Vec2<float>;
using Vec3i = Vec3<int>;
using Vec2i = Vec2<int>;

#endif //TINYRENDERLESSONCODE_MATH_H

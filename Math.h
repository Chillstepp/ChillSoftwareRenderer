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
        T raw[3];
    };
    Vec3(): x(0), y(0), z(0) {}
    Vec3(T _x, T _y, T _z): x(_x), y(_y), z(_z){}

    inline Vec3<T> operator ^(const Vec3<T>& vec){
        return Vec3<T>{y*vec.z - z*vec.y, z*vec.x - x*vec.z, x*vec.y - y*vec.x};
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


template<size_t DimRow, size_t DimCol,typename T>
class Matrix
{
public:
    float raw[DimRow][DimCol];
    Matrix(){};

     float* operator[] (const size_t idx){
        return raw[idx];
    }

    static Matrix<DimRow, DimCol, T> Identity()
    {
        Matrix<DimCol,DimRow, T> Ret;
        static_assert(DimCol == DimRow);
        for(size_t i = 0; i<DimRow; i++)
        {
            for(size_t j = 0; j<DimCol; j++)
            {
                Ret[i][j] = (i==j);
            }
        }
        return Ret;
    }

	template<size_t DimCol2>
    inline Matrix<DimRow, DimCol2, T> operator*(Matrix<DimCol, DimCol2, T> rhs)
    {
         Matrix<DimRow, DimCol2, T> Ret;
         for(size_t i = 0; i < DimRow; i++)
         {
             for(size_t j = 0; j < DimCol2; j++)
             {
                float Ret_ij = 0.0f;
				for(size_t t = 0; t < DimCol; t++)
				{
					Ret_ij += raw[i][t] * rhs.raw[t][j];
				}
				Ret[i][j] = Ret_ij;
             }
         }
		return Ret;
    }

	static Matrix<4,1,T> Embed(const Vec3<T>& InVec)
	{
		Matrix<4,1,T> Ret;
	 	for(int i=0; i<4; i++)
		{
			 Ret[i][0] = (i<3?InVec.raw[i]:1);
		}
		return Ret;
	}
};

using Mat4x4 = Matrix<4,4,float>;


//  P = αA + βB + γC  (α+β+γ=1 && α>0 && β>0 && γ>0) ,then P is in the triangle ABC
//=>P = (1-β-γ)A + βB + γC
//=>0 = (A-P) + β(B-A) + γ(C-A)
//=>0 = PA + βAB + γAC
// So 0 = PA_x + βAB_x + γAC_x,0 = PA_y + βAB_y + γAC_y
//      => 0 = (1,β,γ)dot(PA_x,AB_x,AC_x),  0 = (1,β,γ)dot(PA_y,AB_y,AC_y)
// we consider  (1,β,γ) as a vec, so (1,β,γ) is 正交 with (PA_x,AB_x,AC_x)，(PA_y,AB_y,AC_y) two vector
// so we can (PA_x,AB_x,AC_x) × (PA_y,AB_y,AC_y) => (U_x,U_y,U_z)
// so (1,β,γ) =  (1,U_y/U_x,U_z/U_x), We can easily get (1,alpha,beta) from cross above two vec

static Vec3f barycentric(const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, Vec3i p)
{
    Vec3f U = Vec3f{point1.x - p.x, point2.x-point1.x, point3.x-point1.x} ^
              Vec3f{point1.y - p.y, point2.y-point1.y, point3.y-point1.y};
    if(std::abs(U.x) == 0) //三角形退化为直线
    {
        return Vec3f{1.0,1.0,-1.0};
    }
    return Vec3f{1.0f-1.0f*(U.y+U.z)/U.x, 1.0f*U.y/U.x, 1.0f*U.z/U.x};
}


#endif //TINYRENDERLESSONCODE_MATH_H

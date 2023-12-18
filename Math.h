//
// Created by why on 2023/11/13.
//

#ifndef TINYRENDERLESSONCODE_MATH_H
#define TINYRENDERLESSONCODE_MATH_H
#include <cmath>
#include <vector>
#include "iostream"
#include "TGAImage.h"


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

    inline float norm()
    {
        return std::sqrt(x*x + y*y);
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
    explicit Vec3(TGAColor color): x(color.raw[0]), y(color.raw[1]), z(color.raw[2]){}

    inline Vec3<T> operator ^(const Vec3<T>& vec){
        return Vec3<T>{y*vec.z - z*vec.y, z*vec.x - x*vec.z, x*vec.y - y*vec.x};
    }

    inline Vec3<T> operator +(const Vec3<T>& vec) const {
        return Vec3<T>{x+vec.x, y+vec.y, z+vec.z};
    }

    inline Vec3<T> operator +(const float& f) const {
        return Vec3<T>{x+f, y+f, z+f};
    }

    inline Vec3<T> operator -(const Vec3<T>& vec) const {
        return Vec3<T>{x-vec.x, y-vec.y, z-vec.z};
    }

	inline Vec3<T> operator -() const {
		return Vec3<T>{-x, -y, -z};
	}

    inline Vec3<T> operator *(const float& f) const {
        return Vec3<T>{x*f, y*f, z*f};
    }

    inline Vec3<T> operator /(const float& f) const {
        return Vec3<T>{x/f, y/f, z/f};
    }

    inline Vec3<T> operator *=(const float& f) const {
        return this->operator*(f);
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

template<typename T>
inline Vec3<T> operator *(const float& f, const Vec3<T>& v) {
    return Vec3<T>{v.x*f, v.y*f, v.z*f};
}

template<typename T>
inline Vec3<T> operator +(const float& f, const Vec3<T>& v) {
    return Vec3<T>{v.x+f, v.y+f, v.z+f};
}


using Vec3f = Vec3<float>;
using Vec2f = Vec2<float>;
using Vec3i = Vec3<int>;
using Vec2i = Vec2<int>;


template<size_t DimRow, size_t DimCol,typename T>
class Matrix
{
public:
    T raw[DimRow][DimCol];
    Matrix(){};
	Matrix(std::initializer_list<Vec3f>ColVecs)
	{
        int ColNumber = 0;
        for(auto vec:ColVecs)
        {
            raw[0][ColNumber] = vec.raw[0], raw[1][ColNumber] = vec.raw[1], raw[2][ColNumber] = vec.raw[2];
            ColNumber ++;
        }
	};

    Matrix(std::initializer_list<std::vector<float>>RowVecs)
    {
        int RowNumber = 0;
        for(auto vec:RowVecs)
        {
            for(int j = 0; j < vec.size(); j++)
            {
                raw[RowNumber][j] = vec[j];
            }
            RowNumber ++;
        }
    };

     float* operator[] (const size_t idx){
        return raw[idx];
    }

    float* operator[] (const size_t idx) const {
        return raw[idx];
    }

    Matrix<DimRow, DimCol,float> operator /=(const float d){
         for(int i = 0;i<DimRow;i++)
         {
             for(int j=0;j<DimCol;j++)
             {
                 raw[i][j] /= d;
             }
         }
        return *this;
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

    inline Matrix<DimCol, DimRow, T> Transpose()
    {
        Matrix<DimCol, DimRow, T> RetMat;
        for(int i = 0;i<DimRow;++i)
        {
            for(int j=0;j<DimCol;++j)
            {
                RetMat[j][i] = raw[i][j];
            }
        }
        return RetMat;
    }

    inline Matrix<DimRow, DimCol, T> Inverse(float eps = 1e-3) requires (DimRow == DimCol)
    {
        //[augmentedMatrix, inverseMatrix]构成增广矩阵
        Matrix<DimRow, DimRow, T> augmentedMatrix = *this;
        Matrix<DimRow, DimRow, T> inverseMatrix = Matrix<DimRow, DimRow, T>::Identity();
        for(size_t pivot = 0; pivot<DimRow; pivot++)//处理到第pivot行
        {
            bool FindRow = false;
            for(size_t i = pivot; i<DimRow; i++)
            {
                if(fabs(augmentedMatrix.raw[i][pivot])>eps)
                {
                   FindRow = true;
                   for(int j = 0; j<DimRow; j++)
                   {
                        std::swap(augmentedMatrix.raw[pivot][j], augmentedMatrix.raw[i][j]);
                        std::swap(inverseMatrix.raw[pivot][j], inverseMatrix.raw[i][j]);
                   }
                   break;
                }
            }
            if(!FindRow) throw std::runtime_error("Matrix is not full rank matrix");
            const float Divisor = 1.f/augmentedMatrix.raw[pivot][pivot];
            for(size_t i = 0; i<DimRow; i++)
            {
                augmentedMatrix.raw[pivot][i] *= Divisor;
                inverseMatrix.raw[pivot][i] *= Divisor;
            }
            for(size_t i = pivot + 1; i<DimRow; i++)
            {
                const float factor = augmentedMatrix.raw[i][pivot];
                for(size_t j = 0; j<DimRow; j++)
                {
                    augmentedMatrix.raw[i][j] -= factor * augmentedMatrix.raw[pivot][j];
                    inverseMatrix.raw[i][j] -= factor * inverseMatrix.raw[pivot][j];
                }
            }
        }


        for(int i = DimRow;i >= 0; i--)
        {
            for(int j = i - 1; j >= 0; j--)
            {
                const float factor = augmentedMatrix.raw[j][i];
                for(int k = 0; k < DimRow; k++)
                {
                    augmentedMatrix.raw[j][k] -= factor * augmentedMatrix.raw[i][k];
                    inverseMatrix.raw[j][k] -= factor * inverseMatrix.raw[i][k];
                }
            }
        }
        return inverseMatrix;
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

    static Vec3f Proj(const Matrix<4,1,float>& InMat)
    {
         return Vec3f{InMat.raw[0][0]/InMat.raw[3][0], InMat.raw[1][0]/InMat.raw[3][0], InMat.raw[2][0]/InMat.raw[3][0]};
    }



};

using Mat4x4 = Matrix<4,4,float>;
using Mat3x3 = Matrix<3,3,float>;


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


/**/
//lookat matrix: games 101 is all you need
static Mat4x4 lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (center-eye).normlize();
    Vec3f x = (up^z).normlize();
    Vec3f y = (z^x).normlize();
    Mat4x4 minv = Mat4x4::Identity();//旋转矩阵
    Mat4x4 tr = Mat4x4::Identity();//位移矩阵
    for(int i=0;i<3;i++)
    {
        minv[0][i] = x.raw[i];
        minv[1][i] = y.raw[i];
        minv[2][i] = z.raw[i];
        tr[i][3] = -eye.raw[i];
    }
    return minv * tr;
}

static Mat4x4 viewport(int x, int y, int w, int h)
{
    Mat4x4 ViewPortMat = Mat4x4::Identity();
    ViewPortMat[0][3] = x + w/2.0f;
    ViewPortMat[1][3] = y + h/2.0f;
	//ViewPortMat[2][3] = 1.0f/2.0f;
    ViewPortMat[0][0] = w/2.0f;
    ViewPortMat[1][1] = h/2.0f;
	//ViewPortMat[2][2] = 1.0f/2.0f;
    //it's good to let z [0,255], then we can easily get zbuffer image,
    return ViewPortMat;
}

static Mat4x4 projection(float coeff){
    Mat4x4 Persp2Ortho = Mat4x4::Identity();
    Persp2Ortho[0][0] = 1;
    Persp2Ortho[1][1] = 1;
    Persp2Ortho[2][2] = 1 + 1000;
    Persp2Ortho[2][3] = -1*1000;
	Persp2Ortho[3][2] = 1;

    Mat4x4 Ortho = Mat4x4::Identity();
    Ortho[2][3] = -(1 + 1000)/2.0f;
    return Ortho * Persp2Ortho;
}

//for each point, search around point to find max elevation angle
static float max_elevation_angle(std::vector<std::vector<float>>& zbuffer, Vec2f p, Vec2f dir, float width, float height) {
    float maxangle = 0;
    for (float t=0.; t<100.; t+=1.) {
        Vec2f cur = p + dir*t;
        if (cur.x>=width || cur.y>=height || cur.x<0 || cur.y<0) return maxangle;

        float distance = (p-cur).norm();
        if (distance < 1.f) continue;
        float elevation = zbuffer[int(cur.x)][int(cur.y)]-zbuffer[int(p.x)][int(p.y)];
        maxangle = std::max(maxangle, atanf(-elevation/distance));
    }
    return maxangle;
}


//x,y,z -> roll,pitch,yaw: roll/pitch/yaw rotate around x/y/z
static Mat4x4 RotationMatrix(float roll, float pitch, float yaw)
{
	Mat4x4 rotateAroundX = {
		{1, 0, 0, 0},
		{0, std::cos(roll), -std::sin(roll), 0},
		{0, std::sin(roll),  std::cos(roll), 0},
		{0, 0, 0, 1}
	};

	Mat4x4 rotateAroundY = {
		{ std::cos(pitch), 0,std::sin(pitch), 0},
		{ 0, 1, 0, 0},
		{-std::sin(pitch), 0, std::cos(pitch), 0},
		{ 0, 0, 0, 1}
	};

	Mat4x4 rotateAroundZ = {
		{std::cos(yaw), -std::sin(yaw), 0, 0},
		{std::sin(yaw),  std::cos(yaw), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	};

	return rotateAroundX * rotateAroundY * rotateAroundZ;
}

#endif //TINYRENDERLESSONCODE_MATH_H

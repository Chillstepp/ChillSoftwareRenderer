//
// Created by why on 2023/11/13.
//

#ifndef CHILLSOFTWARERENDERER_MATH_H
#define CHILLSOFTWARERENDERER_MATH_H

#include <cmath>
#include <vector>
#include "iostream"
#include "TGAImage.h"



template<size_t DimRow, size_t DimCol, typename T>
requires std::is_standard_layout_v<T> && std::is_trivial_v<T>
class Matrix;

template<typename T>
struct Vec2 {
    union {
        struct {
            T u, v;
        };
        struct {
            T x, y;
        };
		struct {
			T width, height;
		};
        T raw[2];
    };

    Vec2() : u(0), v(0) {}

    Vec2(T _u, T _v) : u(_u), v(_v) {}

    inline Vec2<T> operator+(const Vec2<T> &vec) const {
        return Vec2<T>{u + vec.u, v + vec.v};
    }

    inline Vec2<T> &operator+=(const Vec2<T> &vec) {
        x = x + vec.x;
        y = y + vec.y;
        return *this;
    }


    inline Vec2<T> operator-(const Vec2<T> &vec) const {
        return Vec2<T>{u - vec.u, v - vec.v};
    }


    inline Vec2<T> operator*(const float &f) const {
        return Vec2<T>{u * f, v * f};
    }

    inline float norm() {
        return std::sqrt(x * x + y * y);
    }

    template<class>
    friend std::ostream &operator<<(std::ostream &s, Vec2<T> &v);

};

template<class T>
std::ostream &operator<<(std::ostream &s, Vec2<T> &v) {
    s << "(" << v.x << "," << v.y << ") \n";
    return s;
}


template<typename T>
struct Vec3 {
    union {
        struct {
            T x, y, z;
        };
        struct {
            T ivert, iuv, inorm;
        };
        struct {
            T alpha, beta, gamma;
        };
        T raw[3];
    };

    Vec3() : x(0), y(0), z(0) {}

    Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

    explicit Vec3(TGAColor color) : x(color.raw[0]), y(color.raw[1]), z(color.raw[2]) {}

    inline Vec3<T> operator^(const Vec3<T> &vec) {
        return Vec3<T>{y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x};
    }

    inline Vec3<T> operator+(const Vec3<T> &vec) const {
        return Vec3<T>{x + vec.x, y + vec.y, z + vec.z};
    }

    inline Vec3<T> &operator+=(const Vec3<T> &vec) {
        x = x + vec.x;
        y = y + vec.y;
        z = z + vec.z;
        return *this;
    }

    inline Vec3<T> operator+(const float &f) const {
        return Vec3<T>{x + f, y + f, z + f};
    }

    inline Vec3<T> &operator+=(const float &f) {
        x = x + f;
        y = y + f;
        z = z + f;
        return *this;
    }

    inline Vec3<T> operator-(const Vec3<T> &vec) const {
        return Vec3<T>{x - vec.x, y - vec.y, z - vec.z};
    }

    inline Vec3<T> operator-() const {
        return Vec3<T>{-x, -y, -z};
    }

    inline Vec3<T> operator*(const float &f) const {
        return Vec3<T>{x * f, y * f, z * f};
    }

    inline Vec3<T> operator/(const float &f) const {
        return Vec3<T>{x / f, y / f, z / f};
    }

    inline std::vector<T> ToStdVector() const {
        return std::vector<T>{raw[0], raw[1], raw[2]};
    }

    inline Matrix<3, 1, T> ToMatrix() const {
        return Matrix<3, 1, T>{{x},
                               {y},
                               {z}};
    }

    inline Vec3<T> &operator*=(const float &f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    inline T operator*(const Vec3<T> &vec) const {
        return x * vec.x + y * vec.y + z * vec.z;
    }

    float norm() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3<T> &normlize(T l = 1) {
        *this = (*this) * (l / norm());
        return *this;
    }

    template<class>
    friend std::ostream &operator<<(std::ostream &s, Vec3<T> &v);

};

template<class T>
std::ostream &operator<<(std::ostream &s, Vec3<T> &v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}

template<typename T>
inline Vec3<T> operator*(const float &f, const Vec3<T> &v) {
    return Vec3<T>{v.x * f, v.y * f, v.z * f};
}

template<typename T>
inline Vec3<T> operator+(const float &f, const Vec3<T> &v) {
    return Vec3<T>{v.x + f, v.y + f, v.z + f};
}


template<typename T>
class Vec4 {
public:
    union {
        struct {
            T x, y, z, w;
        };
        T raw[4];
    };

    Vec4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}

    Vec4() = default;

    inline Vec3<T> ToVec3() const {
        return Vec3{x, y, z};
    }
};

using Vec3f = Vec3<float>;
using Vec2f = Vec2<float>;
using Vec3i = Vec3<int>;
using Vec2i = Vec2<int>;
using Vec4f = Vec4<float>;


template<size_t DimRow, size_t DimCol, typename T>
requires std::is_standard_layout_v<T> && std::is_trivial_v<T>
class Matrix {
public:
    T raw[DimRow][DimCol] = {0};

    Matrix() {

    };

    Matrix(std::initializer_list<std::vector<float>> RowVecs) {
        int RowNumber = 0;
        for (const auto &vec: RowVecs)
        {
            for (int j = 0; j < vec.size(); j++)
            {
                raw[RowNumber][j] = vec[j];
            }
            RowNumber++;
        }
    };

    float *operator[](const size_t idx) {
        return raw[idx];
    }

    float *operator[](const size_t idx) const {
        return raw[idx];
    }

    Matrix<DimRow, DimCol, float> &operator/=(const float d) {
        for (int i = 0; i < DimRow; i++)
        {
            for (int j = 0; j < DimCol; j++)
            {
                raw[i][j] /= d;
            }
        }
        return *this;
    }

    Matrix<DimRow, DimCol, float> &operator*=(const float d) {
        for (int i = 0; i < DimRow; i++)
        {
            for (int j = 0; j < DimCol; j++)
            {
                raw[i][j] *= d;
            }
        }
        return *this;
    }

    static Matrix<DimRow, DimCol, T> Identity() {
        Matrix<DimCol, DimRow, T> Ret;
        static_assert(DimCol == DimRow);
        for (size_t i = 0; i < DimRow; i++)
        {
            for (size_t j = 0; j < DimCol; j++)
            {
                Ret[i][j] = (i == j);
            }
        }
        return Ret;
    }

    template<size_t DimCol2>
    inline Matrix<DimRow, DimCol2, T> operator*(Matrix<DimCol, DimCol2, T> rhs) {
        Matrix<DimRow, DimCol2, T> Ret;
        for (size_t i = 0; i < DimRow; i++)
        {
            for (size_t j = 0; j < DimCol2; j++)
            {
                float Ret_ij = 0.0f;
                for (size_t t = 0; t < DimCol; t++)
                {
                    Ret_ij += raw[i][t] * rhs.raw[t][j];
                }
                Ret[i][j] = Ret_ij;
            }
        }
        return Ret;
    }

    inline Matrix<DimCol, DimRow, T> Transpose() {
        Matrix<DimCol, DimRow, T> RetMat;
        for (int i = 0; i < DimRow; ++i)
        {
            for (int j = 0; j < DimCol; ++j)
            {
                RetMat[j][i] = raw[i][j];
            }
        }
        return RetMat;
    }

    inline Matrix<DimRow, DimCol, T> Normalize() {
        Matrix<DimRow, DimCol, T> RetMat = *this;
        float sum = 0;
        for (int i = 0; i < DimRow; i++)
        {
            for (int j = 0; j < DimCol; j++)
            {
                sum += raw[i][j] * raw[i][j];
            }
        }
        sum = std::sqrt(sum);
        for (int i = 0; i < DimRow; i++)
        {
            for (int j = 0; j < DimCol; j++)
            {
                RetMat[i][j] /= sum;
            }
        }
        return RetMat;
    }

    inline Matrix<DimRow, DimCol, T> Inverse(float eps = 1e-6)
    requires (DimRow == DimCol) {
        //[augmentedMatrix, inverseMatrix]构成增广矩阵
        Matrix<DimRow, DimRow, T> augmentedMatrix = *this;
        Matrix<DimRow, DimRow, T> inverseMatrix = Matrix<DimRow, DimRow, T>::Identity();
        for (size_t pivot = 0; pivot < DimRow; pivot++)//处理到第pivot行
        {
            bool FindRow = false;
            for (size_t i = pivot; i < DimRow; i++)
            {
                if (fabs(augmentedMatrix.raw[i][pivot]) > eps)
                {
                    FindRow = true;
                    for (int j = 0; j < DimRow; j++)
                    {
                        std::swap(augmentedMatrix.raw[pivot][j], augmentedMatrix.raw[i][j]);
                        std::swap(inverseMatrix.raw[pivot][j], inverseMatrix.raw[i][j]);
                    }
                    break;
                }
            }
            if (!FindRow)
            {
                throw std::runtime_error("Matrix is not full rank matrix");
            }
            const float Divisor = 1.f / augmentedMatrix.raw[pivot][pivot];
            for (size_t i = 0; i < DimRow; i++)
            {
                augmentedMatrix.raw[pivot][i] *= Divisor;
                inverseMatrix.raw[pivot][i] *= Divisor;
            }
            for (size_t i = pivot + 1; i < DimRow; i++)
            {
                const float factor = augmentedMatrix.raw[i][pivot];
                for (size_t j = 0; j < DimRow; j++)
                {
                    augmentedMatrix.raw[i][j] -= factor * augmentedMatrix.raw[pivot][j];
                    inverseMatrix.raw[i][j] -= factor * inverseMatrix.raw[pivot][j];
                }
            }
        }


        for (int i = DimRow - 1; i >= 0; i--)
        {
            for (int j = i - 1; j >= 0; j--)
            {
                const float factor = augmentedMatrix.raw[j][i];
                for (int k = 0; k < DimRow; k++)
                {
                    augmentedMatrix.raw[j][k] -= factor * augmentedMatrix.raw[i][k];
                    inverseMatrix.raw[j][k] -= factor * inverseMatrix.raw[i][k];
                }
            }
        }
        return inverseMatrix;
    }

    inline Matrix<DimRow - 1, DimCol - 1, T> RemoveHomogeneousDim() {
        Matrix<DimRow - 1, DimCol - 1, T> Ret;
        for (int i = 0; i < DimRow - 1; i++)
        {
            for (int j = 0; j < DimCol - 1; j++)
            {
                Ret[i][j] = raw[i][j];
            }
        }
        return Ret;
    }

    inline Vec3f ToVec3f()
    requires (DimRow == 3 and DimCol == 1 and std::is_same_v<std::remove_cvref_t<T>, float>) {
        return Vec3f{raw[0][0], raw[1][0], raw[2][0]};
    }

    inline Vec4f ToVec4f()
    requires (DimRow == 4 and DimCol == 1 and std::is_same_v<std::remove_cvref_t<T>, float>) {
        return Vec4f{raw[0][0], raw[1][0], raw[2][0], raw[3][0]};
    }

    static Matrix<4, 1, T> Embed(const Vec3<T> &InVec, T FillValue = 1) {
        Matrix<4, 1, T> Ret;
        for (int i = 0; i < 4; i++)
        {
            Ret[i][0] = (i < 3 ? InVec.raw[i] : FillValue);
        }
        return Ret;
    }

    static Vec3f Proj(const Matrix<4, 1, float> &InMat, bool DoNormalization = true) {
        if (DoNormalization)
        {
            return Vec3f{InMat.raw[0][0] / InMat.raw[3][0], InMat.raw[1][0] / InMat.raw[3][0],
                         InMat.raw[2][0] / InMat.raw[3][0]};
        }
        return Vec3f{InMat.raw[0][0], InMat.raw[1][0], InMat.raw[2][0]};
    }


};

using Mat4x4 = Matrix<4, 4, float>;
using Mat3x3 = Matrix<3, 3, float>;
using Mat4x1 = Matrix<4, 1, float>;
using Mat3x1 = Matrix<3, 1, float>;


//  P = αA + βB + γC  (α+β+γ=1 && α>0 && β>0 && γ>0) ,then P is in the triangle ABC
//=>P = (1-β-γ)A + βB + γC
//=>0 = (A-P) + β(B-A) + γ(C-A)
//=>0 = PA + βAB + γAC
// So 0 = PA_x + βAB_x + γAC_x,0 = PA_y + βAB_y + γAC_y
//      => 0 = (1,β,γ)dot(PA_x,AB_x,AC_x),  0 = (1,β,γ)dot(PA_y,AB_y,AC_y)
// we consider  (1,β,γ) as a vec, so (1,β,γ) is 正交 with (PA_x,AB_x,AC_x)，(PA_y,AB_y,AC_y) two vector
// so we can (PA_x,AB_x,AC_x) × (PA_y,AB_y,AC_y) => (U_x,U_y,U_z)
// so (1,β,γ) =  (1,U_y/U_x,U_z/U_x), We can easily get (1,alpha,beta) from cross above two vec

static Vec3f barycentric(const Vec3f &point1, const Vec3f &point2, const Vec3f &point3, Vec3i p) {
    Vec3f U = Vec3f{point1.x - p.x, point2.x - point1.x, point3.x - point1.x} ^
              Vec3f{point1.y - p.y, point2.y - point1.y, point3.y - point1.y};
    if (std::abs(U.x) == 0) //三角形退化为直线
    {
        return Vec3f{1.0, 1.0, -1.0};
    }
    return Vec3f{1.0f - 1.0f * (U.y + U.z) / U.x, 1.0f * U.y / U.x, 1.0f * U.z / U.x};
}


/**/
//lookat matrix: games 101 is all you need
static Mat4x4 lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (center - eye).normlize();
    Vec3f x = (up ^ z).normlize();
    Vec3f y = (z ^ x).normlize();
    Mat4x4 minv = Mat4x4::Identity();//旋转矩阵
    Mat4x4 tr = Mat4x4::Identity();//位移矩阵
    for (int i = 0; i < 3; i++)
    {
        minv[0][i] = x.raw[i];
        minv[1][i] = y.raw[i];
        minv[2][i] = z.raw[i];
        tr[i][3] = -eye.raw[i];
    }
    return minv * tr;
}

static Mat4x4 viewport(int x, int y, int w, int h) {
    Mat4x4 ViewPortMat = Mat4x4::Identity();
    ViewPortMat[0][3] = x + w / 2.0f;
    ViewPortMat[1][3] = y + h / 2.0f;
    ViewPortMat[2][3] = 255.0f / 2.0f;
    ViewPortMat[0][0] = w / 2.0f;
    ViewPortMat[1][1] = h / 2.0f;
    ViewPortMat[2][2] = 255.0f / 2.0f;
    //it's good to let z [0,255], then we can easily get zbuffer image,
    return ViewPortMat;
}

static Mat4x4 projection(float near = 1, float far = 1000) {

    Mat4x4 Persp2Ortho = Mat4x4::Identity();
    Persp2Ortho[0][0] = 1;
    Persp2Ortho[1][1] = 1;
    Persp2Ortho[2][2] = near + far;
    Persp2Ortho[2][3] = -near * far;
    Persp2Ortho[3][2] = 1;

    Mat4x4 Ortho_Translation = Mat4x4::Identity();
    Mat4x4 Ortho_Scale = Mat4x4::Identity();
    Ortho_Translation[2][3] = -1.0f * (near + far) / 2;
    Ortho_Scale[2][2] = 2.0f / (far - near);

    return Ortho_Scale * Ortho_Translation * Persp2Ortho;
}

//for each point, search around point to find max elevation angle
static float
max_elevation_angle(std::vector<std::vector<float>> &zbuffer, Vec2f p, Vec2f dir, float width, float height) {
    float maxangle = 0;
    for (float t = 0.; t < 100.; t += 1.)
    {
        Vec2f cur = p + dir * t;
        if (cur.x >= width || cur.y >= height || cur.x < 0 || cur.y < 0) return maxangle;

        float distance = (p - cur).norm();
        if (distance < 1.f) continue;
        float elevation = zbuffer[int(cur.x)][int(cur.y)] - zbuffer[int(p.x)][int(p.y)];
        maxangle = std::max(maxangle, atanf(-elevation / distance));
    }
    return maxangle;
}


namespace ChillMathUtility {
    static Vec3f
    TriangleBarycentricInterp(const std::vector<Vec3f> &TriangleVertices3D, const Vec3f &BarycentricCoord) {
        Vec3f p(0, 0, 0);
        for (int i = 0; i < 3; i++)
        {
            p += TriangleVertices3D[i] * BarycentricCoord.raw[i];
        }
        return p;
    }

    static Vec2f
    TriangleBarycentricInterp(const std::vector<Vec2f> &TriangleVertices2D, const Vec3f &BarycentricCoord) {
        Vec2f p(0, 0);
        for (int i = 0; i < 3; i++)
        {
            p += TriangleVertices2D[i] * BarycentricCoord.raw[i];
        }
        return p;
    }

    //x,y,z -> roll,pitch,yaw: roll/pitch/yaw rotate around x/y/z
    static Mat4x4 RotationMatrix(float roll, float pitch, float yaw) {
        Mat4x4 rotateAroundX = {
                {1, 0,              0,               0},
                {0, std::cos(roll), -std::sin(roll), 0},
                {0, std::sin(roll), std::cos(roll),  0},
                {0, 0,              0,               1}
        };

        Mat4x4 rotateAroundY = {
                {std::cos(pitch),  0, std::sin(pitch), 0},
                {0,                1, 0,               0},
                {-std::sin(pitch), 0, std::cos(pitch), 0},
                {0,                0, 0,               1}
        };

        Mat4x4 rotateAroundZ = {
                {std::cos(yaw), -std::sin(yaw), 0, 0},
                {std::sin(yaw), std::cos(yaw),  0, 0},
                {0,             0,              1, 0},
                {0,             0,              0, 1}
        };

        return rotateAroundX * rotateAroundY * rotateAroundZ;
    }

    static float Lerp(float from, float to, float t) {
        return from + (to - from) * t;
    }

    static Vec3f PerspectiveCorrectInterpolation(const std::vector<Vec4f> &HomogeneousCoord, Vec3f LinearInterpBaryCoord) {
        Vec3f CorrectBaryCoord;
        for (int i = 0; i < 3; i++)
        {
            CorrectBaryCoord.raw[i] = LinearInterpBaryCoord.raw[i] / HomogeneousCoord[i].w;
        }

        float Z_n = 1.0f / (CorrectBaryCoord.x + CorrectBaryCoord.y + CorrectBaryCoord.z);
        CorrectBaryCoord *= Z_n;

        return CorrectBaryCoord;
    };

    static Vec3f ReflectedVec(Vec3f InVec, Vec3f Normal)
    {
        return (InVec - Normal * (Normal * InVec * 2.f)).normlize();
    }

    /*!
     * @brief
     * @tparam T: DataType
     * @tparam Dim: AABB Dimension
     * @param pts： pts of 2D-Shape/3D-Volume to get AABB
     * @return AABB (std::pair<<Vec2/Vec3<T>, <Vec2/Vec3<T>>)
     */
    template<typename T, int Dim>
    requires (Dim >= 2) && (Dim <= 3)
    static auto GetAABB(const std::vector<typename std::conditional_t<Dim == 2, Vec2<T>, Vec3<T>>>& pts)
    {
        if constexpr(Dim == 2) {
            using VecType = Vec2<T>;
            VecType bboxmin(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
            VecType bboxmax(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());
            for (int i = 0; i < pts.size(); i++) {
                bboxmin.x = std::min(bboxmin.x, pts[i].x);
                bboxmin.y = std::min(bboxmin.y, pts[i].y);

                bboxmax.x = std::max(bboxmax.x, pts[i].x);
                bboxmax.y = std::max(bboxmax.y, pts[i].y);
            }
            return std::pair<VecType, VecType>(bboxmin, bboxmax);
        }
        else if constexpr(Dim == 3) {
            using VecType = Vec3<T>;
            VecType bboxmin(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
            VecType bboxmax(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest());
            for (int i = 0; i < pts.size(); i++) {
                bboxmin.x = std::min(bboxmin.x, pts[i].x);
                bboxmin.y = std::min(bboxmin.y, pts[i].y);
                bboxmin.z = std::min(bboxmin.z, pts[i].z);

                bboxmax.x = std::max(bboxmax.x, pts[i].x);
                bboxmax.y = std::max(bboxmax.y, pts[i].y);
                bboxmax.z = std::max(bboxmax.z, pts[i].z);
            }
            return std::pair<VecType, VecType>(bboxmin, bboxmax);
        }
    }

    namespace AABBHelper
    {
        template<typename T, int Dim>
        using BoxDataType = std::conditional_t<Dim == 2, Vec2<T>, Vec3<T>>;

        template<typename T, int Dim>
        using BoxType = std::pair<BoxDataType<T, Dim>, BoxDataType<T, Dim>>;
    }

    /*!
     * @brief Clamp AABB out of ClampAABB, usually used for clamp pixel that out of screen
     * @tparam T
     * @tparam Dim
     * @param AABB
     * @param ClampAABB
     * @return
     */
    template<typename T, int Dim>
    requires (Dim >= 2) && (Dim <= 3)
    static auto GetAABB_Intersect(const AABBHelper::BoxType<T, Dim>& AABB1, const AABBHelper::BoxType<T, Dim>& AABB2, bool& bIntersect)
    {
        AABBHelper::BoxType<T, Dim> ResultBox;
        for(int i = 0; i < Dim; i++)
        {
            ResultBox.first.raw[i] = std::max(AABB1.first.raw[i], AABB2.first.raw[i]);
            ResultBox.second.raw[i] = std::min(AABB1.second.raw[i], AABB2.second.raw[i]);
        }
        bIntersect = true;
        for(int i = 0; i < Dim; i++)
        {
            bIntersect &= (ResultBox.first.raw[i] > ResultBox.second.raw[i]);
        }
        return ResultBox;
    }

}

#endif //CHILLSOFTWARERENDERER_MATH_H

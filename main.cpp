#include <iostream>
#include "TGAImage.h"
#include "Model.h"

const TGAColor White = TGAColor{255,255,255,255};
const TGAColor red = TGAColor{255,0,0,255};
constexpr int width  = 800; // output image size
constexpr int height = 800;
std::vector<std::vector<float>>ZBuffer(width,std::vector<float>(height, -std::numeric_limits<float>::max()));
Vec3f LightDir{0,0,-1};
Model* model = nullptr;


void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1-x0;
    int dy = y1-y0;
    int derror2 = std::abs(dy)*2;
    int error2 = 0;
    int y = y0;
    for (int x=x0; x<=x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1>y0?1:-1);
            error2 -= dx*2;
        }
    }
}

//  P = αA + βB + γC  (α+β+γ=1 && α>0 && β>0 && γ>0) ,then P is in the triangle ABC
//=>P = (1-β-γ)A + βB + γC
//=>0 = (A-P) + β(B-A) + γ(C-A)
//=>0 = PA + βAB + γAC
// So 0 = PA_x + βAB_x + γAC_x,0 = PA_y + βAB_y + γAC_y
//      => 0 = (1,β,γ)dot(PA_x,AB_x,AC_x),  0 = (1,β,γ)dot(PA_y,AB_y,AC_y)
// we consider  (1,β,γ) as a vec, so (1,β,γ) is 正交 with (PA_x,AB_x,AC_x)，(PA_y,AB_y,AC_y) two vector
// so we can (PA_x,AB_x,AC_x) × (PA_y,AB_y,AC_y) => (U_x,U_y,U_z)
// so (1,β,γ) =  (1,U_y/U_x,U_z/U_x), We can easily get (1,alpha,beta) from cross above two vec

Vec3f barycentric(const Vec3i& point1, const Vec3i& point2, const Vec3i& point3, Vec3i p)
{
    Vec3i U = Vec3i{point1.x - p.x, point2.x-point1.x, point3.x-point1.x} ^
              Vec3i{point1.y - p.y, point2.y-point1.y, point3.y-point1.y};
    if(std::abs(U.x) == 0) //三角形退化为直线
    {
        return Vec3f{1.0,1.0,-1.0};
    }
    return Vec3f{1.0f-1.0f*(U.y+U.z)/U.x, 1.0f*U.y/U.x, 1.0f*U.z/U.x};
}



void triangle(Vec3i *pts, Vec2f* textures, TGAImage &image)
{
    Vec2i bboxmin(image.get_width()-1,  image.get_height()-1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    Vec3i P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            float z = 0;
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            Vec2f PTexture(0,0);
            for (int i = 0; i < 3; i++) {
                z += pts[i].z * bc_screen.raw[i];//插值得到三角形是某点的z值
//                std::cout<<pts[i].z << bc_screen.raw[i]<<std::endl;
            }
            for (int i = 0; i < 3; i++) {
                PTexture.u += textures[i].u * bc_screen.raw[i];
                PTexture.v += textures[i].v * bc_screen.raw[i];
            }
            //在三角形外部则跳过这个点继续循环
            if(ZBuffer[P.x][P.y] < z)
            {
                ZBuffer[P.x][P.y] = z;
                auto Color = model->diffuse(PTexture);
                image.set(P.x, P.y, Color);
            }
        }
    }

}



int main(int argc, char** argv) {
    auto head = "../obj/african_head/african_head.obj";
    auto boogie = "../obj/boggie/body.obj";
    auto diablo = "../obj/diablo3_pose/diablo3_pose.obj";
    model = new Model(head);
    TGAImage image{width,height,TGAImage::RGB};
    for(int i=0;i<model->nfaces();i++)
    {
        const std::vector<int>& face = model->getface(i);
        Vec3i ScreenCoords[3];
        Vec3f WorldCoords[3];
        Vec2f Textures[3];
        for(int j=0;j<3;j++)
        {
            WorldCoords[j] = model->getvert(face[j*2]);
            ScreenCoords[j] = {static_cast<int>((WorldCoords[j].x+1.0f)*width/2.0f),
                               static_cast<int>((WorldCoords[j].y+1.0f)*height/2.0f),
                               static_cast<int>(WorldCoords[j].z*10000)};
            Textures[j] = model->getuv(face[j*2+1]);
        }
//        Vec3f norm = (WorldCoords[2] - WorldCoords[0])^(WorldCoords[1] - WorldCoords[0]);//obj文件 面顶点为逆时针顺序
//        norm.normlize();
//        float intensity = norm * LightDir;

        triangle(ScreenCoords, Textures, image);
    }

    image.flip_vertically();//left-bottom is the origin
    image.write_tga_file("output.tga");
    return 0;
}

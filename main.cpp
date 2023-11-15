#include <iostream>
#include "TGAImage.h"
#include "Model.h"
#include "Math.h"
#include "Render.h"

const TGAColor White = TGAColor{255,255,255,255};
const TGAColor red = TGAColor{255,0,0,255};
auto head = "../obj/african_head/african_head.obj";
auto boogie = "../obj/boggie/body.obj";
auto diablo = "../obj/diablo3_pose/diablo3_pose.obj";


constexpr int width  = 800; // output image size
constexpr int height = 800;
Vec3f LightDir{0,0,-1};
Vec3f Eye{0,0,3};
Vec3f Center{0,0,0};
Vec3f Up{0,1,0};

Mat4x4 ModelView;
Mat4x4 ViewPort;
Mat4x4 Projection;

//lookat matrix: games 101 is all you need
Mat4x4 lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye-center).normlize();
    Vec3f x = (up^z).normlize();
    Vec3f y = (z^x).normlize();
    Mat4x4 minv = Mat4x4::Identity();//旋转矩阵
    Mat4x4 tr = Mat4x4::Identity();//位移矩阵
    for(int i=0;i<3;i++)
    {
        minv[0][i] = x.raw[i];
        minv[1][i] = y.raw[i];
        minv[2][i] = z.raw[i];
        tr[i][3] = -center.raw[i];
    }
    return minv * tr;
}

int main(int argc, char** argv) {

    Model* model = new Model(head);
    std::vector<std::vector<float>>ZBuffer(width,std::vector<float>(height, -std::numeric_limits<float>::max()));
    TGAImage image{width,height,TGAImage::RGB};
    for(int i=0;i<model->nfaces();i++)
    {
        Mat4x4::Identity();
        const std::vector<int>& face = model->getface(i);
        Vec3i ScreenCoords[3];
        Vec3f WorldCoords[3];
        Vec2f Textures[3];
        for(int j=0;j<3;j++)
        {
            WorldCoords[j] = model->getvert(face[j*2]);
            ScreenCoords[j] = {static_cast<int>((WorldCoords[j].x+1.0f)*width/2.0f),
                               static_cast<int>((WorldCoords[j].y+1.0f)*height/2.0f),
                               static_cast<int>(WorldCoords[j].z*100000)};
            Textures[j] = model->getuv(face[j*2+1]);
        }
//        Vec3f norm = (WorldCoords[2] - WorldCoords[0])^(WorldCoords[1] - WorldCoords[0]);//obj文件 面顶点为逆时针顺序
//        norm.normlize();
//        float intensity = norm * LightDir;

        triangle(model,ScreenCoords, Textures, image,ZBuffer);
    }

    image.flip_vertically();//left-bottom is the origin
    image.write_tga_file("output.tga");
    return 0;
}

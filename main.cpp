#include <iostream>
#include <memory>
#include "TGAImage.h"
#include "Model.h"
#include "Math.h"
#include "Render.h"
#include "Shader/IShader.h"

const TGAColor White = TGAColor{255,255,255,255};
const TGAColor red = TGAColor{255,0,0,255};
auto head = "../obj/african_head/african_head.obj";
auto boogie = "../obj/boggie/body.obj";
auto diablo = "../obj/diablo3_pose/diablo3_pose.obj";


constexpr int width  = 800; // output image size
constexpr int height = 800;
Vec3f LightDir{1,1,1};
Vec3f Eye{0,-1,3};
Vec3f Center{0,0,0};
Vec3f Up{0,1,0};


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

Mat4x4 viewport(int x, int y, int w, int h)
{
	Mat4x4 ViewPortMat = Mat4x4::Identity();
	ViewPortMat[0][3] = x + w/2.0f;
	ViewPortMat[1][3] = y + h/2.0f;
	ViewPortMat[0][0] = w/2.0f;
	ViewPortMat[1][1] = h/2.0f;
	//it's good to let z [0,255], then we can easily get zbuffer image,
	return ViewPortMat;
}

Mat4x4 projection(float coeff){
	Mat4x4 Projection = Mat4x4::Identity();
	Projection[3][2] = coeff;
	return Projection;
}

Vec3f world2screenCoord(Vec3f v, Mat4x4 modelview, Mat4x4 projection, Mat4x4 viewport)
{
	auto gl_vertex = Mat4x4::Embed(v);
	gl_vertex = viewport * projection * modelview * gl_vertex;
	float d = gl_vertex.raw[3][0];
	//std::cout<<gl_vertex.raw[0][0]<<" "<<gl_vertex.raw[1][0]<<" "<<gl_vertex.raw[2][0]<<" "<<gl_vertex.raw[3][0]<<std::endl;
	return {
		gl_vertex.raw[0][0]/d,
		gl_vertex.raw[1][0]/d,
		gl_vertex.raw[2][0]/d
	};
}

Mat4x4 ModelView = lookat(Eye, Center, Up);
Mat4x4 ViewPort = viewport(0, 0, width, height);
Mat4x4 Projection = projection(-1.0f/3.0f);
TGAImage image{width,height,TGAImage::RGB};
std::vector<std::vector<float>>ZBuffer(width,std::vector<float>(height, -99999));

int main(int argc, char** argv) {
    std::shared_ptr<Model> model = std::make_shared<Model>(diablo);
	//FlatShader* flatShader = new FlatShader(model, Projection, ModelView, ViewPort, LightDir);
    //GouraudShader* gouraudShader = new GouraudShader(model, Projection, ModelView, ViewPort, LightDir);
    std::shared_ptr<IShader> Shader = std::make_shared<PhongShader>(model, Projection, ModelView, ViewPort, LightDir);

    for(int i=0;i<model->nfaces();i++)
    {
        const std::vector<int>& face = model->getface(i);
        Vec3f ScreenCoords[3];
        Vec3f WorldCoords[3];
        Vec2f Textures[3];
        for(int j=0;j<3;j++)
        {
            WorldCoords[j] = model->getvert(face[j*3]);
			auto Mat4x1_Vertex = Shader->vertex(i, j);
			ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0]};
            Textures[j] = model->getuv(face[j*3+1]);
        }
        triangle(model,ScreenCoords, Textures, image,ZBuffer,Shader);
    }

    image.flip_vertically();//left-bottom is the origin
    image.write_tga_file("output.tga");
    return 0;
}

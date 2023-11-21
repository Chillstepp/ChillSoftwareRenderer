#include <iostream>
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
Vec3f LightDir{0,0,-1};
Vec3f Eye{0,0,3};
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

int main(int argc, char** argv) {

    Model* model = new Model(diablo);
	//FlatShader* flatShader = new FlatShader(model, Projection, ModelView, ViewPort, LightDir);
    //GouraudShader* gouraudShader = new GouraudShader(model, Projection, ModelView, ViewPort, LightDir);
    PhongShader* phongShader = new PhongShader(model, Projection, ModelView, ViewPort, LightDir);
    std::vector<std::vector<float>>ZBuffer(width,std::vector<float>(height, -std::numeric_limits<float>::max()));
    TGAImage image{width,height,TGAImage::RGB};
    for(int i=0;i<model->nfaces();i++)
    {
        Mat4x4::Identity();
        const std::vector<int>& face = model->getface(i);
        Vec3f ScreenCoords[3];
        Vec3f WorldCoords[3];
        Vec2f Textures[3];
        for(int j=0;j<3;j++)
        {
            WorldCoords[j] = model->getvert(face[j*3]);
			auto Mat4x1_Vertex = phongShader->vertex(i, j);
			ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0]};
			//ScreenCoords[j] = world2screenCoord(WorldCoords[j], ModelView, Projection, ViewPort);
//            ScreenCoords[j] = {static_cast<int>((WorldCoords[j].x+1.0f)*width/2.0f),
//                               static_cast<int>((WorldCoords[j].y+1.0f)*height/2.0f),
//                               static_cast<int>(WorldCoords[j].z*100000)};
            Textures[j] = model->getuv(face[j*3+1]);
        }
//        Vec3f norm = (WorldCoords[2] - WorldCoords[0])^(WorldCoords[1] - WorldCoords[0]);//obj文件 面顶点为逆时针顺序
//        norm.normlize();
//        float intensity = norm * LightDir;
        triangle(model,ScreenCoords, Textures, image,ZBuffer,phongShader);
    }

    image.flip_vertically();//left-bottom is the origin
    image.write_tga_file("output.tga");

	delete model;
	//delete flatShader;
    //delete gouraudShader;
    delete phongShader;
    return 0;
}

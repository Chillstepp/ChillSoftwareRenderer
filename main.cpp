#include <iostream>
#include <memory>
#include "TGAImage.h"
#include "Model.h"
#include "Math.h"
#include "Render.h"
#include "Shader/IShader.h"
#include "Window.h"

auto head = "../obj/african_head/african_head.obj";
auto boogie = "../obj/boggie/body.obj";
auto diablo = "../obj/diablo3_pose/diablo3_pose.obj";


constexpr int width  = 800; // output image size
constexpr int height = 800;
Vec3f LightDir{1,1,1};
Vec3f Eye{0,-1,3};
Vec3f Center{0,0,0};
Vec3f Up{0,1,0};


Mat4x4 ModelView = lookat(Eye, Center, Up);
Mat4x4 ViewPort = viewport(0, 0, width, height);
Mat4x4 Projection = projection(-1.0f/3.0f);
std::vector<std::vector<float>>ZBuffer(width,std::vector<float>(height, -std::numeric_limits<float>::max()));
std::vector<std::vector<float>>DepthBuffer(width,std::vector<float>(height, -std::numeric_limits<float>::max()));

int main(int argc, char** argv) {
    std::shared_ptr<Model> model = std::make_shared<Model>(diablo);
	//FlatShader* Shader = new FlatShader(model, Projection, ModelView, ViewPort, LightDir);
    //GouraudShader* Shader = new GouraudShader(model, Projection, ModelView, ViewPort, LightDir);


    std::shared_ptr<IShader> Shader_dep = std::make_shared<DepthShder>(model, projection(0), lookat(LightDir, Center, Up), ViewPort);
    TGAImage image{width,height,TGAImage::RGB};
    for(int i=0;i<model->nfaces();i++)
    {
        const std::vector<int>& face = model->getface(i);
        Vec3f ScreenCoords[3];
        Vec2f Textures[3];
        for(int j=0;j<3;j++)
        {
            auto Mat4x1_Vertex = Shader_dep->vertex(i, j);
            ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0]};
            Textures[j] = model->getuv(face[j*3+1]);
        }
        triangle(model,ScreenCoords, Textures, image,DepthBuffer,Shader_dep);
    }
    image.flip_vertically();//left-bottom is the origin
    image.write_tga_file("output_depth.tga");


    Mat4x4 Uniform_MShadow = (ViewPort*Projection*ModelView)*(ViewPort*projection(0)*lookat(LightDir, Center, Up)).Inverse();
    std::shared_ptr<IShader> Shader = std::make_shared<PhongShader>(model, Projection, ModelView, ViewPort, LightDir, Uniform_MShadow, DepthBuffer);
    TGAImage image2{width,height,TGAImage::RGB};
    for(int i=0;i<model->nfaces();i++)
    {
        const std::vector<int>& face = model->getface(i);
        Vec3f ScreenCoords[3];
        Vec2f Textures[3];
        for(int j=0;j<3;j++)
        {
			auto Mat4x1_Vertex = Shader->vertex(i, j);
			ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0]};
            Textures[j] = model->getuv(face[j*3+1]);
        }
        triangle(model,ScreenCoords, Textures, image2,ZBuffer,Shader);
    }
    image2.flip_vertically();//left-bottom is the origin
    image2.write_tga_file("output.tga");



    std::shared_ptr<IShader> AOShader = std::make_shared<SSAOShader>(model, Projection, ModelView, ViewPort);
    TGAImage image3{width,height,TGAImage::RGB};
    for(int i=0;i<model->nfaces();i++)
    {
        const std::vector<int>& face = model->getface(i);
        Vec3f ScreenCoords[3];
        Vec2f Textures[3];
        for(int j=0;j<3;j++)
        {
            auto Mat4x1_Vertex = AOShader->vertex(i, j);
            ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0]};
            Textures[j] = model->getuv(face[j*3+1]);
        }
        triangle(model,ScreenCoords, Textures, image3,ZBuffer,AOShader);
    }

    //post-process : SSAO
    for (int x=0; x<width; x++) {
        for (int y = 0; y < height; y++) {
            if(ZBuffer[x][y] < -1e5) continue;
            float total = 0;
            for(float angle = 0; angle<M_PI*2-(1e-4); angle += M_PI/4)
            {
                total += M_PI/2.0f - max_elevation_angle(ZBuffer, Vec2f(x,y), Vec2f(cos(angle), sin(angle)), width, height);
            }
            total /= (M_PI/2)*8;
            total = std::pow(total, 100.0f);
            image3.set(x, y, TGAColor(total*255, total*255, total*255, 255));
        }
        //std::cout<<x<<std::endl;
    }
    image3.flip_vertically();//left-bottom is the origin
    image3.write_tga_file("AO.tga");

    return 0;
}

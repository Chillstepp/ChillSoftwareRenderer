#include <iostream>
#include <memory>
#include "TGAImage.h"
#include "Model.h"
#include "Math.h"
#include "Render.h"
#include "Shader/IShader.h"
#include "Shader/PhongShader.h"
#include "Window.h"
#include "Scene.h"

namespace FilePath
{
	auto head = "../obj/african_head/african_head.obj";
	auto boogie = "../obj/boggie/body.obj";
	auto diablo = "../obj/diablo3_pose/diablo3_pose.obj";
	auto floor = "../obj/floor.obj";
}


constexpr int width  = 2000; // output image size
constexpr int height = 2000;
Vec3f LightDir{-1.5,-1.5,-1.5};
Vec3f LightSpotLoc = -LightDir;
Vec3f Eye{0,0,1.5};
Vec3f Center{0,0,0};
Vec3f Up{0,1,0};


Mat4x4 ModelView = lookat(Eye, Center, Up);
Mat4x4 ViewPort = viewport(0, 0, width, height);
Mat4x4 Projection = projection(1.0f/3.0f);
std::vector<std::vector<float>>ZBuffer(width,std::vector<float>(height, std::numeric_limits<float>::max()));
std::vector<std::vector<float>>DepthBuffer(width,std::vector<float>(height, std::numeric_limits<float>::max()));
std::vector<std::vector<float>>ShadowBuffer(width, std::vector<float>(height, 0));
std::vector<std::vector<float>>PenumbraBuffer(width,std::vector<float>(height, 0));

int main(int argc, char** argv) {

	std::unique_ptr<Scene> scene = std::make_unique<Scene>();
    std::shared_ptr<Model> model_diablo = std::make_shared<Model>(FilePath::diablo);
	std::shared_ptr<Model> model_floor = std::make_shared<Model>(FilePath::floor);
	scene->Add(model_floor);
	scene->Add(model_diablo);



	//FlatShader* Shader = new FlatShader(model, Projection, ModelView, ViewPort, LightDir);
    //GouraudShader* Shader = new GouraudShader(model, Projection, ModelView, ViewPort, LightDir);

    TGAImage image{width,height,TGAImage::RGB};
	TGAImage image2{width,height,TGAImage::RGB};
	TGAImage image3{width,height,TGAImage::RGB};
	for(auto& model_WeakPtr: scene->GetAllModels())
	{
		auto model = model_WeakPtr.lock();

		std::shared_ptr<IShader> Shader_dep =
			std::make_shared<DepthShder>(model, projection(1.0f / 3.0f), lookat(LightSpotLoc, Center, Up), ViewPort);
		for (int i = 0; i < model->nfaces(); i++)
		{
			const std::vector<int>& face = model->getface(i);
			Vec3f ScreenCoords[3];
			for (int j = 0; j < 3; j++)
			{
				auto Mat4x1_Vertex = Shader_dep->vertex(i, j);
				ScreenCoords[j] = { Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0] };
			}
			triangle(model, ScreenCoords, image, DepthBuffer, Shader_dep);
		}
	}

	for(auto& model_WeakPtr: scene->GetAllModels())
	{
		auto model = model_WeakPtr.lock();
		Mat4x4 Uniform_MShadow = (ViewPort*projection(1.0f/3.0f)*lookat(LightSpotLoc, Center, Up)) * (ModelView).Inverse();
		std::shared_ptr<IShader> Shader = std::make_shared<PhongShader>(model, Projection, ModelView, ViewPort,
                                                                        LightDir, Eye, Center, Uniform_MShadow, DepthBuffer, ShadowBuffer, PenumbraBuffer);
		for(int i=0;i<model->nfaces();i++)
		{
			const std::vector<int>& face = model->getface(i);
            Vec3f WorldCoords[3];
            for(int j=0;j<3;j++)
            {
                WorldCoords[j] = model->getvert(i, j);
            }
			Vec3f ScreenCoords[3];
			for(int j=0;j<3;j++)
			{
				auto Mat4x1_Vertex = Shader->vertex(i, j);
				ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0]};
			}
            Vec3f tri_normal = (WorldCoords[2]-WorldCoords[0])^(WorldCoords[1]-WorldCoords[0]);
            if(tri_normal.normlize() * (WorldCoords[0] - Eye) >= 0) //back face culling
            {
                triangle(model,ScreenCoords, image2,ZBuffer,Shader);
            }
		}


//		std::shared_ptr<IShader> AOShader = std::make_shared<SSAOShader>(model, Projection, ModelView, ViewPort);
//		for(int i=0;i<model->nfaces();i++)
//		{
//			const std::vector<int>& face = model->getface(i);
//			Vec3f ScreenCoords[3];
//			for(int j=0;j<3;j++)
//			{
//				auto Mat4x1_Vertex = AOShader->vertex(i, j);
//				ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0]};
//			}
//
//			triangle(model,ScreenCoords, image3,ZBuffer,AOShader);
//		}
	}
	image.flip_vertically();
	image.write_tga_file("lightview_depth.tga");

    //Shadow: PCF

//    int sampleHalfSize = 3;
//    int sampleSize = 2*sampleHalfSize + 1;
//    for(int i = sampleHalfSize; i < width - sampleHalfSize; i++)
//    {
//        for(int j = sampleHalfSize; j < height - sampleHalfSize; j++)
//        {
//           // if(!ShadowBuffer[i][j]) continue;
//            int SampleShadowNumber = 0;
//            for(int dx = -sampleHalfSize; dx <= sampleHalfSize; dx++)
//            {
//                for(int dy = -sampleHalfSize; dy <= sampleHalfSize; dy++)
//                {
//                    SampleShadowNumber += ShadowBuffer[i + dx][j + dy];
//                }
//            }
//            float SampleShadowRate = 1.0f*SampleShadowNumber / (sampleSize * sampleSize);
//            float PCFShadowFactor = 1.0f - 0.7f*SampleShadowRate;
//            image2.set(i ,j, image2.get(i,j) * PCFShadowFactor);
//        }
//    }


    //Shadow: PCSS
    for(int i = 0; i < width; i++)
    {
        for(int j = 0; j < height; j++)
        {
            int sampleHalfSize = PenumbraBuffer[i][j];
            int sampleSize = 2*sampleHalfSize + 1;
            int SampleShadowNumber = 0;
            //std::cout<<sampleHalfSize<<"\n";
            if(i - sampleHalfSize >= 0 && i + sampleHalfSize < width && j - sampleHalfSize >= 0 && j + sampleHalfSize < height)
            {
                for(int dx = -sampleHalfSize; dx <= sampleHalfSize; dx++)
                {
                    for(int dy = -sampleHalfSize; dy <= sampleHalfSize; dy++)
                    {
                        SampleShadowNumber += ShadowBuffer[i + dx][j + dy];
                    }
                }
                float SampleShadowRate = 1.0f*SampleShadowNumber / (sampleSize * sampleSize);
                float PCFShadowFactor = 1.0f - 0.7f*SampleShadowRate;
                image2.set(i ,j, image2.get(i,j) * PCFShadowFactor);
            }
        }
    }

    //Post-process : ACES ToneMapping
    for (int x=0; x<width; x++) {
        for (int y = 0; y < height; y++) {
            TGAColor color = image2.get(x, y);
            ACESToneMapping({(float)color.raw[0], (float)color.raw[1], (float)color.raw[2]});
        }
    }

	image2.flip_vertically();//left-bottom is the origin
	image2.write_tga_file("output.tga");

    //post-process : SSAO
    for (int x=0; x<width; x++) {
        for (int y = 0; y < height; y++) {
            if(ZBuffer[x][y] < -1e5) continue;
            float total = 0;
            for(float angle = 0; angle<M_PI*2.0f-(1e-4); angle += M_PI/4)
            {
                total += M_PI/2.0f - max_elevation_angle(ZBuffer, Vec2f(x,y), Vec2f(cos(angle), sin(angle)), width, height);
            }
            total /= (M_PI/2)*8;
            total = std::pow(total, 100.0f);
            image3.set(x, y, TGAColor(total*255, total*255, total*255, 255));
        }
    }


    image3.flip_vertically();//left-bottom is the origin
    image3.write_tga_file("AO.tga");

    return 0;
}

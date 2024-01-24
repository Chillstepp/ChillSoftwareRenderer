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
#include "Shader/GBuffer.h"
#include "random"

namespace FilePath {
    auto head = "../obj/african_head/african_head.obj";
    auto boogie = "../obj/boggie/body.obj";
    auto diablo = "../obj/diablo3_pose/diablo3_pose.obj";
    auto floor = "../obj/floor.obj";
    auto gun = "../obj/gun/Cerberus.obj";
    auto helmet = "../obj/helmet/helmet.obj";
}

constexpr int width = 2000; // output image size
constexpr int height = 2000;
Vec3f LightDir{-1.5, -1.5, -1.5};
Vec3f LightSpotLoc = -LightDir;
Vec3f Eye{0, 0, 1.5};
Vec3f Center{0, 0, 0};
Vec3f Up{0, 1, 0};


Mat4x4 ModelView = lookat(Eye, Center, Up);
Mat4x4 ViewPort = viewport(0, 0, width, height);
Mat4x4 Projection = projection(1.0f / 3.0f);
std::vector<std::vector<float>> ZBuffer(width, std::vector<float>(height, std::numeric_limits<float>::max()));
std::vector<std::vector<float>> DepthBuffer(width, std::vector<float>(height, std::numeric_limits<float>::max()));
std::vector<std::vector<float>> ShadowBuffer(width, std::vector<float>(height, 0));
std::vector<std::vector<float>> PenumbraBuffer(width, std::vector<float>(height, 0));
std::vector<std::vector<Vec3f>> NormalBuffer(width, std::vector<Vec3f>(height, Vec3f(0, 0, 0)));
std::vector<std::vector<Mat3x3>> TBNBuffer(width, std::vector<Mat3x3>(height, Mat3x3()));
std::vector<std::vector<Vec3f>> ScreenPosWBuffer(width, std::vector<Vec3f>(height, Vec3f(0, 0, 0)));//归一化坐标

std::random_device rd;
std::mt19937 RandomGen(rd());
std::uniform_real_distribution<float> UniformDis01(0.0f, 1.0f);

int main(int argc, char **argv) {

    std::unique_ptr<Scene> scene = std::make_unique<Scene>();
    std::shared_ptr<Model> model_diablo = std::make_shared<Model>(FilePath::diablo);
    std::shared_ptr<Model> model_floor = std::make_shared<Model>(FilePath::floor);
    scene->Add(model_floor);
    scene->Add(model_diablo);



    //FlatShader* Shader = new FlatShader(model, Projection, ModelView, ViewPort, LightDir);
    //GouraudShader* Shader = new GouraudShader(model, Projection, ModelView, ViewPort, LightDir);

    TGAImage image{width, height, TGAImage::RGB};
    TGAImage image2{width, height, TGAImage::RGB};
    TGAImage image3{width, height, TGAImage::RGB};
    for (auto &model_WeakPtr: scene->GetAllModels()) {
        auto model = model_WeakPtr.lock();

        std::shared_ptr<IShader> Shader_dep =
                std::make_shared<DepthShder>(model, projection(1.0f / 3.0f), lookat(LightSpotLoc, Center, Up),
                                             ViewPort);
        for (int i = 0; i < model->nfaces(); i++) {
            const std::vector<int> &face = model->getface(i);
            std::vector<Vec4f> ScreenCoords{3, {0, 0, 0, 0}};
            for (int j = 0; j < 3; j++) {
                auto Mat4x1_Vertex = Shader_dep->vertex(i, j);
                ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0],
                                   Mat4x1_Vertex.raw[3][0]};
            }
            triangle(model, ScreenCoords, image, DepthBuffer, Shader_dep);
        }
    }

    for (auto &model_WeakPtr: scene->GetAllModels()) {
        auto model = model_WeakPtr.lock();
        Mat4x4 Uniform_MShadow =
                (ViewPort * projection(1.0f / 3.0f) * lookat(LightSpotLoc, Center, Up)) * (ModelView).Inverse();
        std::shared_ptr<IShader> Shader = std::make_shared<PhongShader>(model, Projection, ModelView, ViewPort,
                                                                        LightDir, Eye, Center, Uniform_MShadow,
                                                                        DepthBuffer, ShadowBuffer, PenumbraBuffer,
                                                                        NormalBuffer, TBNBuffer, ScreenPosWBuffer);
        for (int i = 0; i < model->nfaces(); i++) {
            const std::vector<int> &face = model->getface(i);
            Vec3f WorldCoords[3];
            for (int j = 0; j < 3; j++) {
                WorldCoords[j] = model->getvert(i, j);
            }
            std::vector<Vec4f> ScreenCoords{3, {0, 0, 0, 0}};
            for (int j = 0; j < 3; j++) {
                auto Mat4x1_Vertex = Shader->vertex(i, j);
                ScreenCoords[j] = {Mat4x1_Vertex.raw[0][0], Mat4x1_Vertex.raw[1][0], Mat4x1_Vertex.raw[2][0],
                                   Mat4x1_Vertex.raw[3][0]};
            }
            Vec3f tri_normal = (WorldCoords[2] - WorldCoords[0]) ^ (WorldCoords[1] - WorldCoords[0]);
            if (tri_normal.normlize() * (WorldCoords[0] - Eye) >= 0) //back face culling
            {
                triangle(model, ScreenCoords, image2, ZBuffer, Shader);
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
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int sampleHalfSize = PenumbraBuffer[i][j];
            int sampleSize = 2 * sampleHalfSize + 1;
            int SampleShadowNumber = 0;
            //std::cout<<sampleHalfSize<<"\n";
            if (i - sampleHalfSize >= 0 && i + sampleHalfSize < width && j - sampleHalfSize >= 0 &&
                j + sampleHalfSize < height) {
                for (int dx = -sampleHalfSize; dx <= sampleHalfSize; dx++) {
                    for (int dy = -sampleHalfSize; dy <= sampleHalfSize; dy++) {
                        SampleShadowNumber += ShadowBuffer[i + dx][j + dy];
                    }
                }
                float SampleShadowRate = 1.0f * SampleShadowNumber / (sampleSize * sampleSize);
                float PCFShadowFactor = 1.0f - 0.7f * SampleShadowRate;
                image2.set(i, j, image2.get(i, j) * PCFShadowFactor);
            }
        }
    }

    //Post-process : ACES ToneMapping
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            TGAColor color = image2.get(x, y);
            ACESToneMapping({(float) color.raw[0], (float) color.raw[1], (float) color.raw[2]});
        }
    }

    image2.flip_vertically();//left-bottom is the origin
    image2.write_tga_file("output.tga");

    //post-process : SSAO
    Mat4x4 WorldSpaceMat2ScreenSpace = ViewPort * Projection * ModelView;
    Mat4x4 ScreenSpace2WorldSpaceMat = (Projection * ModelView).Inverse();
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (ZBuffer[x][y] > 1e5) continue;
            Mat4x1 ScreenSpaceCoord = Mat4x1::Embed(
                    Vec3f((x - 1.0f * width / 2) / (1.0f * width / 2),
                          (y - 1.0f * height / 2) / (1.0f * height / 2),
                          (ZBuffer[x][y] - 255.0f / 2.0f) / (255.0f / 2.0f)));
            Mat4x1 WorldCoord = ScreenSpace2WorldSpaceMat * ScreenSpaceCoord;
            WorldCoord /= WorldCoord.raw[3][0];
            Vec3f n = NormalBuffer[x][y];
            if (n.norm() == 0) continue;

            int Count = 0;
            int Total = 0;
            int SampleTimes = 64;
            float randR = 0.070f;
            for (int i = 0; i < SampleTimes; i++) {
                Vec3f SamplePoint = Vec3f{UniformDis01(RandomGen) * 2.0f - 1.0f, UniformDis01(RandomGen) * 2.0f - 1.0f,
                                          UniformDis01(RandomGen)};
                Vec3f RandomVec = Vec3f{UniformDis01(RandomGen) * 2.0f - 1.0f, UniformDis01(RandomGen) * 2.0f - 1.0f,
                                        UniformDis01(RandomGen) * 2.0f - 1.0f};
                Vec3f tangent = (RandomVec - NormalBuffer[x][y] *
                                             (RandomVec * NormalBuffer[x][y])).normlize();//Gramm-Schmidt Process
                Vec3f bitangent = NormalBuffer[x][y] ^ tangent;
                Mat3x3 TBN{
                        {tangent.x, bitangent.x, NormalBuffer[x][y].x},
                        {tangent.y, bitangent.y, NormalBuffer[x][y].y},
                        {tangent.z, bitangent.z, NormalBuffer[x][y].z}
                };


                Vec3f SampleVec = SamplePoint;
                SampleVec = (TBN * SampleVec.ToMatrix()).ToVec3f().normlize();
                float scale = 1.0f * i / SampleTimes;
                scale = std::lerp(0.1f, 1.0f, scale * scale);
                SampleVec *= scale;
                Mat4x1 SampleWorldCoord{
                        {WorldCoord.raw[0][0] + SampleVec.raw[0]},
                        {WorldCoord.raw[1][0] + SampleVec.raw[1]},
                        {WorldCoord.raw[2][0] + SampleVec.raw[2]},
                        {1.0f}
                };


                Mat4x1 SamplePointInScreenSpace = WorldSpaceMat2ScreenSpace * SampleWorldCoord;
                SamplePointInScreenSpace /= SamplePointInScreenSpace.raw[3][0];
                Vec2i ScreenXY(SamplePointInScreenSpace.raw[0][0], SamplePointInScreenSpace.raw[1][0]);

                if (ScreenXY.x < width and ScreenXY.x >= 0 and ScreenXY.y >= 0 and ScreenXY.y < height) {
                    if (ZBuffer[ScreenXY.x][ScreenXY.y] <= SamplePointInScreenSpace.raw[2][0]) Count++;//occu
                    Total++;
                }
            }
            TGAColor c = TGAColor(255, 255, 255, 255) * (1 - 1.0f * Count / SampleTimes);
            image3.set(x, y, c);
        }
    }

    image3.flip_vertically();//left-bottom is the origin
    image3.write_tga_file("AO.tga");

    return 0;
}

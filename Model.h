//
// Created by why on 2023/11/13.
//

#ifndef CHILLSOFTWARERENDERER_MODEL_H
#define CHILLSOFTWARERENDERER_MODEL_H

#include "vector"
#include "Math.h"
#include "TGAImage.h"
#include "unordered_map"

enum class EFaceOrientation : uint8_t {
	Unknown,
    Right,
    Left,
    Top,
    Bottom,
    Front,
    Back
};

class Model {
private:
    std::vector<Vec3f> Verts;
    std::vector<std::vector<int>> Faces;
    std::vector<Vec3f> Norms;
    std::vector<Vec2f> uvs;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;

	//pbr use
	TGAImage AOMap;
	TGAImage MetallicMap;
	TGAImage RoughnessMap;
	TGAImage AlbedoMap;
    TGAImage EmissionMap;
	TGAImage brdfLUTMap;

	std::unordered_map<EFaceOrientation, TGAImage*> SkyBoxDiffuseMaps;

    void load_texture(std::string filename, const char *suffix, TGAImage &img);


public:
    Model(const char *filename);

    ~Model();

    int nverts();

    int nfaces();

    Vec3f getvert(int iface, int nthvert);

    std::vector<int> getface(int idx);

    Vec2f getuv(int iface, int nthvert);

    //Get vertex-normal in obj file
    Vec3f getNormal(int iface, int nthvert);

    //Get normal in normal map
    Vec3f getNormal(Vec2f uvf);

    //Specular Map
    float getSpecular(Vec2f uvf);

    //Specular Map: Return RGB Color(Color from 0.0 to 1.0)
    Vec3f getSpecular_RGB(Vec2f uvf);

    TGAColor diffuse(Vec2f uv);

	Vec3f getAO(Vec2f uvf);
	Vec3f getMetallic(Vec2f uvf);
	Vec3f getRoughness(Vec2f uvf);
	Vec3f getAlbedo(Vec2f uvf);
    Vec3f getEmission(Vec2f uvf);
	Vec3f getLUT(Vec2f uvf);

    TGAColor diffuseSkyBox(Vec2f uv, EFaceOrientation FaceOrientation);

    void loadSkyboxTexture(const std::string& filename);


private:
    Vec2f getuv(int idx);

    Vec3f getvert(int i);
};


#endif //CHILLSOFTWARERENDERER_MODEL_H

//
// Created by why on 2023/11/13.
//

#include <sstream>
#include "Model.h"
#include "fstream"

Model::Model(const char *filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) {
        throw std::runtime_error("fail to load model");
        return;
    }
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v.raw[i];
            Verts.push_back(v);
        } else if (!line.compare(0, 2,
                                 "f ")) { //f 24/1/24 25/2/25 26/3/26 : vertex idx,vertexTexture idx,vertex normal vector idx
            std::vector<int> f;
            int itrash, idx, idxt, idxn;
            iss >> trash;//f
            while (iss >> idx >> trash >> idxt >> trash >> idxn) {
                idx--;//obj format idx from 0
                idxt--;
                idxn--;
                f.push_back(idx);
                f.push_back(idxt);
                f.push_back(idxn);
            }
            Faces.push_back(f);
        } else if (!line.compare(0, 2, "vt")) {
            iss >> trash >> trash;
            Vec2f uv;
            iss >> uv.u >> uv.v;
            uvs.push_back(uv);
        } else if (!line.compare(0, 2, "vn")) {
            iss >> trash >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v.raw[i];
            Norms.push_back(v);
        }
    }
    load_texture(filename, "_diffuse.tga", diffusemap_);
    load_texture(filename, "_nm_tangent.tga", normalmap_);
    load_texture(filename, "_spec.tga", specularmap_);
}

Model::~Model() {

}

int Model::nverts() {
    return Verts.size();
}

int Model::nfaces() {
    return Faces.size();
}

Vec3f Model::getvert(int i) {
    return Verts.at(i);
}

Vec3f Model::getvert(int iface, int nthvert) {
    const std::vector<int> &Face = getface(iface);
    return Verts.at(Face[nthvert * 3]);
}


std::vector<int> Model::getface(int idx) {
    return Faces.at(idx);
}

Vec2f Model::getuv(int idx) {
    Vec2f uv = uvs.at(idx);
    uv.u = fmodf(uv.u, 1.0000001);
    uv.v = fmodf(uv.v, 1.0000001);
    return uv;
}

Vec2f Model::getuv(int iface, int nthvert) {
    const std::vector<int> &Face = getface(iface);
    Vec2f uv = getuv(Face[nthvert * 3 + 1]);
    return uv;
}


TGAColor Model::diffuse(Vec2f uv) {
    Vec2i uvwh(uv.u * diffusemap_.get_width(), uv.v * diffusemap_.get_height());
    return diffusemap_.get(uvwh.raw[0], uvwh.raw[1]);
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &img) {
    std::string textfile{filename};
    size_t dot = textfile.find_last_of(".");
    if (dot != std::string::npos) {
        textfile = textfile.substr(0, dot) + std::string(suffix);
        img.read_tga_file(textfile.c_str());
        img.flip_vertically();
    }
}

void Model::loadSkyboxTexture(const std::string& filename)
{
	std::string textfile(filename);
	size_t dot = textfile.find_last_of(".");
	if (dot != std::string::npos) {
        TGAImage* img_top = new TGAImage;
        std::string suffix = "_top.tga";
		textfile = textfile.substr(0, dot) + suffix;
        img_top->read_tga_file(textfile.c_str());
        img_top->flip_vertically();
        SkyBoxDiffuseMaps[EFaceOrientation::Top] = img_top;

        TGAImage* img_bottom = new TGAImage;
        suffix = "_bottom.tga";
        textfile = textfile.substr(0, dot) + suffix;
        img_bottom->read_tga_file(textfile.c_str());
        img_bottom->flip_vertically();
        SkyBoxDiffuseMaps[EFaceOrientation::Bottom] = img_bottom;

        TGAImage* img_left = new TGAImage;
        suffix = "_left.tga";
        textfile = textfile.substr(0, dot) + suffix;
        img_left->read_tga_file(textfile.c_str());
        img_left->flip_vertically();
        SkyBoxDiffuseMaps[EFaceOrientation::Left] = img_left;

        TGAImage* img_right = new TGAImage;
        suffix = "_right.tga";
        textfile = textfile.substr(0, dot) + suffix;
        img_right->read_tga_file(textfile.c_str());
        img_right->flip_vertically();
        SkyBoxDiffuseMaps[EFaceOrientation::Right] = img_right;

        TGAImage* img_front = new TGAImage;
        suffix = "_front.tga";
        textfile = textfile.substr(0, dot) + suffix;
        img_front->read_tga_file(textfile.c_str());
        img_front->flip_vertically();
        SkyBoxDiffuseMaps[EFaceOrientation::Front] = img_front;

        TGAImage* img_back = new TGAImage;
        suffix = "_back.tga";
        textfile = textfile.substr(0, dot) + suffix;
        img_back->read_tga_file(textfile.c_str());
        img_back->flip_vertically();
        SkyBoxDiffuseMaps[EFaceOrientation::Back] = img_back;

	}
}

Vec3f Model::getNormal(int iface, int nthvert) {

    const std::vector<int> &face = getface(iface);
    auto norm = Norms.at(face[nthvert * 3 + 2]).normlize();
    return norm;
}

Vec3f Model::getNormal(Vec2f uvf) {
    Vec2i uv(uvf.u * normalmap_.get_width(), uvf.v * normalmap_.get_height());
    TGAColor normal_color = normalmap_.get(uv.u, uv.v);
    Vec3f res;
    //TGAColor is bgra, and in byte
    for (int i = 0; i < 3; i++) {
        res.raw[2 - i] = normal_color.raw[i] / 255.0f * 2.0f - 1.0f;// map to [-1,1]
    }
    return res;
}

float Model::getSpecular(Vec2f uvf) {
    Vec2i uv(uvf.u * specularmap_.get_width(), uvf.v * specularmap_.get_height());
    TGAColor specular_color = specularmap_.get(uv.u, uv.v);
    return specular_color.raw[0] / 1.0f;
}

Vec3f Model::getSpecular_RGB(Vec2f uvf) {
    Vec2i uv(uvf.u * specularmap_.get_width(), uvf.v * specularmap_.get_height());
    TGAColor specular_color = specularmap_.get(uv.u, uv.v);
    Vec3f res;
    for (int i = 0; i < 3; i++) {
        res.raw[2 - i] = specular_color.raw[i] / 255.0f * 2.0f - 1.0f;// map to [-1,1]
    }
    return res;
}

TGAColor Model::diffuseSkyBox(Vec2f uv, EFaceOrientation FaceOrientation) {
    if(FaceOrientation == EFaceOrientation::Unknown)
    {
        std::cout<<"why"<<"\n";
    }
    TGAImage* DiffMap = SkyBoxDiffuseMaps[FaceOrientation];
	Vec2i uvwh(uv.u * DiffMap->get_width(), uv.v * DiffMap->get_height());
	return DiffMap->get(uvwh.raw[0], uvwh.raw[1]);
}




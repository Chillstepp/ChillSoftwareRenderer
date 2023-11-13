//
// Created by why on 2023/11/13.
//

#include <sstream>
#include "Model.h"
#include "fstream"

Model::Model(const char *filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if(in.fail()) {
        throw std::runtime_error("fail to load model");
        return;
    }
    std::string line;
    while(!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if(!line.compare(0, 2, "v ")){
            iss>>trash;
            Vec3f v;
            for(int i=0;i<3;i++) iss>>v.raw[i];
            Verts.push_back(v);
        }
        else if(!line.compare(0, 2, "f ")){ //f 24/1/24 25/2/25 26/3/26
            std::vector<int> f;
            int itrash, idx;
            iss>>trash;//f
            while(iss>>idx>>trash>>itrash>>trash>>itrash){ //只读24 25 26
                idx--;//obj format idx from 0
                f.push_back(idx);
            }
            Faces.push_back(f);
        }
    }
}

Model::~Model() {

}

int Model::nverts() {
    return Verts.size();
}

int Model::nfaces() {
    return Faces.size();
}

Vec3f Model::getvert(int i){
    return Verts.at(i);
}

std::vector<int> Model::getface(int idx){
    return Faces.at(idx);
}

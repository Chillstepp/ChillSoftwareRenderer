//
// Created by why on 2023/12/6.
//

#ifndef CHILLSOFTWARERENDERER_SCENE_H
#define CHILLSOFTWARERENDERER_SCENE_H

#include "memory"
#include "vector"
#include "Model.h"

class Scene {
public:
    Scene() = default;

    Scene(std::initializer_list<std::shared_ptr<Model>> ModelList) {
        for (const auto &model: ModelList) {
            Models.push_back(model);
        }
    }

    void Add(const std::shared_ptr<Model> &AddModel);

    decltype(auto) GetAllModels() {
        return Models;
    }

    //@todo: Wrap Light class.
    Vec3f LightDir{-1.5, -1.5, -1.5};
    Vec3f LightPos{1.5, 1.5, 1.5};

    //@todo: Distinguish spot light and sky light
    void SetLightDir(Vec3f LightDir_)
    {
        LightDir = LightDir_;
        LightPos = -LightDir_;
    }
private:
    std::vector<std::weak_ptr<Model>> Models;
};


#endif //CHILLSOFTWARERENDERER_SCENE_H

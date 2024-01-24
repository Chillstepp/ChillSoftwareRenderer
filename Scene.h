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

private:
    std::vector<std::weak_ptr<Model>> Models;
};


#endif //CHILLSOFTWARERENDERER_SCENE_H

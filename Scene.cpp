//
// Created by why on 2023/12/6.
//

#include "Scene.h"

void Scene::Add(const std::shared_ptr<Model> &AddModel) {
    Models.push_back(AddModel);
}

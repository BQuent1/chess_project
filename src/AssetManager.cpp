#include "AssetManager.hpp"

void AssetManager::loadModel(const std::string& name, const std::string& path) {
    if (_models.find(name) == _models.end()) {
        Model m;
        if (path.find(".glb") != std::string::npos || path.find(".gltf") != std::string::npos) {
            m.loadFromGLTF(path);
        } else {
            m.loadFromObj(path);
        }
        _models[name] = std::move(m);
        std::cout << "AssetManager : Modèle chargé -> " << name << std::endl;
    } else {
        std::cout << "AssetManager : Le modèle " << name << " est déjà en cache." << std::endl;
    }
}

const Model* AssetManager::getModel(const std::string& name) const {
    auto it = _models.find(name);
    if (it != _models.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool AssetManager::hasModel(const std::string& name) const {
    return _models.find(name) != _models.end();
}

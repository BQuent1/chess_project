#pragma once

#include <string>
#include <map>
#include <iostream>
#include "Model.hpp"

class AssetManager {
private:
    std::map<std::string, Model> _models;

public:
    AssetManager() = default;
    ~AssetManager() = default;

    // Charger un modèle et l'associer à une clé
    void loadModel(const std::string& name, const std::string& path);

    // Récupérer un modèle
    const Model* getModel(const std::string& name) const;

    // Vérifier si un modèle est chargé
    bool hasModel(const std::string& name) const;
};

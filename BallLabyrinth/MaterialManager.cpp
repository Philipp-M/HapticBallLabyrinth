//
// Created by steve on 1/10/17.
//

#include "MaterialManager.hpp"

MaterialManager::MaterialManager() {}

MaterialManager&
MaterialManager::getInstance()
{
    static MaterialManager instance;
    return instance;
}

void
MaterialManager::addMaterial(std::shared_ptr<Material> material)
{
    materials.push_back(material);
}

const std::shared_ptr<Material>
MaterialManager::getById(int id) const
{
    return materials.size() <= id ? nullptr : materials[id];
}

const std::shared_ptr<Material>
MaterialManager::getByName(const std::string& name) const
{
    for (const auto& m : materials)
    {
        if (m->name == name)
        {
            return m;
        }
    }
    return nullptr;
}

#pragma once

#include "cgltf.h"
#include "device.hpp"
#include "sceneV2/scene.hpp"
#include <filesystem>
#include <string>
namespace TTe {
class GLTFLoader {
    public:
    GLTFLoader(Device *device) : device(device) {}
    void load(const std::filesystem::path &filePath);
    Scene* getScene() const {return scene;}
    
    private:
    void loadMesh(cgltf_data* data);
    void loadMaterial(cgltf_data* data);
    void loadTexture(cgltf_data* data);
    void loadNode(cgltf_data* data);

    std::vector<bool> isAlbedoTex;
    std::filesystem::path dataPath;

    Scene *scene = nullptr;
    Device *device = nullptr;
};
}
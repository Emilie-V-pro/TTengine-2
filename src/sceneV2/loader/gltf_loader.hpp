#pragma once

#include <filesystem>

#include "cgltf.h"
#include "device.hpp"
#include "sceneV2/scene.hpp"

namespace TTe {
class GLTFLoader {
    public:
    GLTFLoader(Device *p_device) : m_device(p_device) {}
    void load(const std::filesystem::path &filePath);
    Scene* getScene() const {return m_scene;}
    
    private:
    void loadMesh(cgltf_data* p_data);
    void loadMaterial(cgltf_data* p_data);
    void loadTexture(cgltf_data* p_data);
    void loadNode(cgltf_data* p_data);

    std::vector<bool> m_is_albedo_tex;
    std::filesystem::path m_data_path;

    Scene *m_scene = nullptr;
    Device *m_device = nullptr;
};
}
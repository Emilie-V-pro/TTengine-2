#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "GPU_data/image.hpp"
#include "sceneV2/i_object_file_loader.hpp"
#include "struct.hpp"
namespace TTe {

class ObjLoader : public IObjectFileLoader {
   public:
   ObjLoader(Device *device) : IObjectFileLoader(device) {}
    virtual ObjectFileData loadObject(std::string objectPath);



    private:
    int find_object( const char *name );
    bool read_materials_mtl(const char *filename, std::vector<Material> &materials);

    int insert_texture(const char *name);
    std::vector<std::vector<unsigned>>  groups(const std::vector<int> &properties);

    std::vector<std::string> object_names;

    std::vector<int> object_indices;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::vector<Image> textures;

};
}  // namespace TTe
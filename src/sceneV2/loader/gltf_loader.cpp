
#include "gltf_loader.hpp"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#define DATA_PATH "../data/"
namespace TTe {

void GLTFLoader::load(const std::string& filePath) {
    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, (DATA_PATH+filePath).c_str(), &data);
    if (result == cgltf_result_success) {
        /* TODO make awesome stuff */
        
        cgltf_free(data);
    }
}

}  // namespace TTe
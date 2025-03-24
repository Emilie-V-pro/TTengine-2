#pragma once

#include <string>
#include <vector>
#include "GPU_data/image.hpp"
#include "device.hpp"
#include "scene/mesh.hpp"
#include "scene/object.hpp"
#include "struct.hpp"
namespace TTe {

    struct ObjectFileData {
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
        std::vector<Image> images;

        ObjectFileData() {};

        //copy and move constructors
        ObjectFileData(const ObjectFileData &other) {
            this->meshes = other.meshes;
            this->materials = other.materials;
            this->images = other.images;
        }
        ObjectFileData &operator=(const ObjectFileData &other) {
            if (this != &other) {
                this->meshes = other.meshes;
                this->materials = other.materials;
                this->images = other.images;
            }
            return *this;
        }

        ObjectFileData(ObjectFileData &&other) {
            this->meshes = std::move(other.meshes);
            this->materials = std::move(other.materials);
            this->images = std::move(other.images);
        }

        ObjectFileData &operator=(ObjectFileData &&other) {
            if (this != &other) {
                this->meshes = std::move(other.meshes);
                this->materials = std::move(other.materials);
                this->images = std::move(other.images);
            }
            return *this;
        }
    };

    class IObjectFileLoader {
    public:
         IObjectFileLoader(Device * device) {
            this->device = device;
        };
         virtual ObjectFileData loadObject(std::string ObjectPath) = 0;

         Device* device;
    };
}
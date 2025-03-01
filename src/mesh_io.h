
#ifndef _MESH_IO_H
#define _MESH_IO_H

#include <vector>
#include "struct.hpp"
#include "utils.hpp"



namespace TTe {


struct MeshIOGroup
{
    int id;
    unsigned first;
    unsigned count;
};

struct MeshIOData
{
    std::vector<Vertex> vertices;

    std::vector<unsigned> indices;
    
    std::vector<Material> materials;
    std::vector<int> material_indices;
    
    std::vector<std::string> object_names;
    std::vector<int> object_indices;
  
    int find_object( const char *name );
    std::vector<MeshIOGroup> sort_by_material( ) { return groups(material_indices); }
    std::vector<MeshIOGroup> sort_by_object( ) { return groups(object_indices); }
    std::vector<MeshIOGroup> groups( const std::vector<int>& properties );
};

bool read_meshio_data( const char *filename, MeshIOData& data );


///@}

#endif
}
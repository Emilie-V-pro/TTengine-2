#pragma once

#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "command_buffer.hpp"

namespace TTe {
class CommandPoolHandler {
   public:
    static CommandBufferPool *getCommandPool(Device *p_device, const VkQueue &p_queue) {
        std::thread::id this_id = std::this_thread::get_id();
        if (s_command_pools.find({this_id, p_queue}) == s_command_pools.end()) {
            s_command_pools[{this_id, p_queue}] = new CommandBufferPool(p_device, p_queue);
        }
        return s_command_pools[{this_id, p_queue}];
    }

    static void cleanUnusedPools(){
        std::vector<std::pair<std::thread::id, VkQueue>> to_delete;
        for (auto &it : s_command_pools) {
            if( it.second->cmd_buffer_count == 0){
                delete it.second;
                to_delete.push_back(it.first);
            }
        }
        for(auto &it : to_delete){
            s_command_pools.erase(it);
        }
    }

    static void destroyCommandPools() {
        for (auto &it : s_command_pools) {
            
            delete it.second;
            
        }
        s_command_pools.clear();
    }
     struct PairHash {
        template <typename T1, typename T2>
        std::size_t operator()(const std::pair<T1, T2> &pair) const {
            auto hash1 = std::hash<T1>{}(pair.first);
            auto hash2 = std::hash<T2>{}(pair.second);
            // Combiner les deux valeurs de hachage
            return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
        }
    };
    static std::unordered_map<std::pair<std::thread::id, VkQueue>, CommandBufferPool *, PairHash> s_command_pools;
   private:
   
};
}  // namespace TTe
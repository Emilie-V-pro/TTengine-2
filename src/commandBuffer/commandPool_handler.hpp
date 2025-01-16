#pragma once

#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "command_buffer.hpp"

namespace TTe {
class CommandPoolHandler {
   public:
    static CommandBufferPool *getCommandPool(const Device *device, const VkQueue &queue) {
        std::thread::id this_id = std::this_thread::get_id();
        if (commandPools.find({this_id, queue}) == commandPools.end()) {
            commandPools[{this_id, queue}] = new CommandBufferPool(device, queue);
        }
        return commandPools[{this_id, queue}];
    }

    static void cleanUnusedPools(){
        std::vector<std::pair<std::thread::id, VkQueue>> toDelete;
        for (auto &it : commandPools) {
            if( it.second->getNBCmBuffers() == 0){
                delete it.second;
                toDelete.push_back(it.first);
            }
        }
        for(auto &it : toDelete){
            commandPools.erase(it);
        }
    }

    static void destroyCommandPools() {
        for (auto &it : commandPools) {
            delete it.second;
        }
        commandPools.clear();
    }

   private:
    struct PairHash {
        template <typename T1, typename T2>
        std::size_t operator()(const std::pair<T1, T2> &pair) const {
            auto hash1 = std::hash<T1>{}(pair.first);
            auto hash2 = std::hash<T2>{}(pair.second);
            // Combiner les deux valeurs de hachage
            return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
        }
    };
    static std::unordered_map<std::pair<std::thread::id, VkQueue>, CommandBufferPool *, PairHash> commandPools;
};
}  // namespace TTe
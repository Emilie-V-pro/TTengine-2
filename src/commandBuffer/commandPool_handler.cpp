
#include "commandPool_handler.hpp"

namespace TTe {



std::unordered_map<std::pair<std::thread::id, VkQueue>, CommandBufferPool*, CommandPoolHandler::PairHash> CommandPoolHandler::s_command_pools;

}